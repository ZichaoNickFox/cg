#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "glog/logging.h"
#include "imgui.h"
#include "implot.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "base/color.h"
#include "base/debug.h"
#include "playground/playground.h"
#include "renderer/gl.h"
#include "renderer/io.h"
#include "rhi/device.h"

#if defined(CG_HAS_MESA_EGL)
#include <EGL/egl.h>
#include <EGL/eglext.h>

#ifndef EGL_CONTEXT_OPENGL_PROFILE_MASK
#define EGL_CONTEXT_OPENGL_PROFILE_MASK 0x30FD
#endif

#ifndef EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT
#define EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT 0x00000001
#endif

#ifndef EGL_PLATFORM_SURFACELESS_MESA
#define EGL_PLATFORM_SURFACELESS_MESA 0x31DD
#endif
#endif

#if defined(CG_HAS_VULKAN_RHI)
#include "rhi/vulkan/presenter.h"
#endif

namespace {

constexpr float kImGuiScale = 1.0f;
constexpr int kDefaultWindowWidth = 1600;
constexpr int kDefaultWindowHeight = 900;

enum class RuntimeMode {
  kOpenGL = 0,
  kVulkan = 1,
};

enum class OpenGLRuntime {
  kNative = 0,
  kMesa = 1,
};

enum class OffscreenBackend {
  kHiddenNativeOpenGL = 0,
  kMesaOSMesa = 1,
  kMesaEGL = 2,
};

struct WindowConfig {
  int width = kDefaultWindowWidth;
  int height = kDefaultWindowHeight;
  bool fullscreen = false;
};

void glfw_error_callback(int error, const char* description) {
  fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

OpenGLRuntime ChooseOpenGLRuntime() {
  const char* requested_gl_runtime = std::getenv("CG_GL_RUNTIME");
  if (requested_gl_runtime != nullptr && std::string(requested_gl_runtime) == "native") {
    return OpenGLRuntime::kNative;
  }
  if (requested_gl_runtime != nullptr && std::string(requested_gl_runtime) == "mesa") {
#if defined(CG_HAS_MESA_OFFSCREEN)
    return OpenGLRuntime::kMesa;
#else
    return OpenGLRuntime::kNative;
#endif
  }
#if defined(CG_HAS_MESA_OFFSCREEN)
  return OpenGLRuntime::kMesa;
#else
  return OpenGLRuntime::kNative;
#endif
}

bool IsMesaOpenGLRuntimeExplicitlyRequested() {
  const char* requested_gl_runtime = std::getenv("CG_GL_RUNTIME");
  return requested_gl_runtime != nullptr && std::string(requested_gl_runtime) == "mesa";
}

bool ShouldUseMesaOpenGLRuntime() {
  return ChooseOpenGLRuntime() == OpenGLRuntime::kMesa;
}

const char* PresenterGLSLVersion() {
#if defined(CG_PLATFORM_MACOS)
  return "#version 410 core";
#else
  return "#version 450 core";
#endif
}

std::string SceneRendererApiName() {
  return cg::rhi::GetSceneApiName();
}

std::string RuntimeRhiApiName() {
  const cg::rhi::Capabilities& caps = cg::rhi::GetCapabilities();
  return caps.graphics_api_name.empty() ? "Unknown" : caps.graphics_api_name;
}

void LogSceneCapabilitySummary() {
  const cg::rhi::Capabilities& caps = cg::rhi::GetCapabilities();
  CGLOG(ERROR) << "Scene Capabilities : GLSL450=" << (caps.supports_glsl_450 ? "yes" : "no")
               << " Compute=" << (caps.supports_compute ? "yes" : "no")
               << " StorageBuffers=" << (caps.supports_storage_buffers ? "yes" : "no");
}

std::string JoinMessages(const std::vector<std::string>& messages) {
  std::string result;
  for (size_t i = 0; i < messages.size(); ++i) {
    if (i > 0) {
      result += "; ";
    }
    result += messages[i];
  }
  return result;
}

void SetEnvIfUnset(const char* name, const char* value) {
  const char* current = std::getenv(name);
  if (current == nullptr || current[0] == '\0') {
    setenv(name, value, 1);
  }
}

#if defined(CG_HAS_MESA_EGL)
void ConfigureMesaLlvmPipeEnvironment() {
  SetEnvIfUnset("LIBGL_ALWAYS_SOFTWARE", "1");
  SetEnvIfUnset("GALLIUM_DRIVER", "llvmpipe");
  SetEnvIfUnset("MESA_LOADER_DRIVER_OVERRIDE", "llvmpipe");
  SetEnvIfUnset("EGL_PLATFORM", "surfaceless");
}

std::string FormatEglErrorMessage(const char* step) {
  char buffer[128] = {};
  std::snprintf(buffer, sizeof(buffer), "%s failed (EGL error 0x%04x)", step, eglGetError());
  return buffer;
}
#endif

std::optional<int> ReadIntEnv(const char* name) {
  const char* raw = std::getenv(name);
  if (raw == nullptr || raw[0] == '\0') {
    return std::nullopt;
  }
  char* end = nullptr;
  const long value = std::strtol(raw, &end, 10);
  if (end == nullptr || *end != '\0') {
    return std::nullopt;
  }
  return static_cast<int>(value);
}

WindowConfig BuildWindowConfig() {
  WindowConfig config;
  if (const std::optional<int> width = ReadIntEnv("CG_WINDOW_WIDTH")) {
    config.width = std::max(1, *width);
  }
  if (const std::optional<int> height = ReadIntEnv("CG_WINDOW_HEIGHT")) {
    config.height = std::max(1, *height);
  }
  return config;
}

void FillIoInput(GLFWwindow* window, ImGuiIO* imgui_io, cg::Io* io) {
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    io->FeedKeyInput("w");
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    io->FeedKeyInput("s");
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    io->FeedKeyInput("a");
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    io->FeedKeyInput("d");
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    io->FeedKeyInput("esc");

  double xpos = 0.0;
  double ypos = 0.0;
  glfwGetCursorPos(window, &xpos, &ypos);
  io->FeedCursorPos({xpos, ypos});

  const int left_button_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1);
  const int right_button_state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2);
  io->FeedButtonInput(left_button_state == GLFW_PRESS, right_button_state == GLFW_PRESS);
  io->SetGuiCapturedMouse(imgui_io->WantCaptureMouse);
}

void UpdateIoSizes(GLFWwindow* window, Playground* playground) {
  glm::ivec2 framebuffer_size;
  glfwGetFramebufferSize(window, &framebuffer_size.x, &framebuffer_size.y);
  playground->mutable_io()->SetFramebufferSize(framebuffer_size);

  glm::ivec2 screen_size;
  glfwGetWindowSize(window, &screen_size.x, &screen_size.y);
  playground->mutable_io()->SetScreenSize(screen_size);
}

void ApplyViewportFromIo(const cg::Io& io) {
  const glm::ivec2 framebuffer_size = io.framebuffer_size();
  if (framebuffer_size.x > 0 && framebuffer_size.y > 0) {
    cg::rhi::GetDevice().SetViewport({0, 0}, framebuffer_size);
  }
}

RuntimeMode ChooseRuntimeMode() {
  const char* requested_runtime = std::getenv("CG_RUNTIME");
  if (requested_runtime != nullptr) {
    const std::string runtime_value = requested_runtime;
    if (runtime_value == "opengl" || runtime_value == "mesa") {
      return RuntimeMode::kOpenGL;
    }
#if defined(CG_HAS_VULKAN_RHI)
    if (runtime_value == "vulkan") {
      return RuntimeMode::kVulkan;
    }
  }
  return RuntimeMode::kVulkan;
#else
  return RuntimeMode::kOpenGL;
#endif
}

GLFWwindow* CreateVisibleOpenGLWindow(const WindowConfig& config, const char* title) {
  glfwDefaultWindowHints();
#if defined(CG_PLATFORM_MACOS)
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#else
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
#endif
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  GLFWmonitor* monitor = config.fullscreen ? glfwGetPrimaryMonitor() : nullptr;
  GLFWwindow* window = glfwCreateWindow(config.width, config.height, title, monitor, nullptr);
  CGCHECK(window != nullptr) << "GLFW create window failed";
  return window;
}

class OffscreenGlRenderer {
 public:
  struct Frame {
    int width = 0;
    int height = 0;
    const std::uint8_t* rgba_pixels = nullptr;
    std::size_t size_in_bytes = 0;
  };

  OffscreenGlRenderer(GLFWwindow* input_window, int width, int height, RuntimeMode runtime_mode)
      : input_window_(CGCHECK_NOTNULL(input_window)) {
    CreateOffscreenContext(width, height);

    MakeCurrent();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    imgui_io_ = &ImGui::GetIO();
    ImPlot::CreateContext();

    ImGui_ImplGlfw_InitForOther(input_window_, true);
    ImGui_ImplOpenGL3_Init(glsl_version_);

    if (runtime_mode == RuntimeMode::kVulkan) {
      cg::rhi::InitializeVulkanDevice(true);
    } else {
      cg::rhi::InitializeOpenGLDevice();
    }
    CGLOG(ERROR) << "Offscreen GL Runtime : " << runtime_name();
    CGLOG(ERROR) << "RHI Runtime API : " << RuntimeRhiApiName();
    CGLOG(ERROR) << "Scene Renderer API : " << SceneRendererApiName();
    LogSceneCapabilitySummary();
  }

  ~OffscreenGlRenderer() {
    MakeCurrent();
    ImPlot::DestroyContext();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    if (offscreen_window_ != nullptr) {
      glfwDestroyWindow(offscreen_window_);
    }
#if defined(CG_HAS_MESA_EGL)
    DestroyMesaEglContext();
#endif
  }

  Frame RenderFrame(Playground* playground) {
    MakeCurrent();

    UpdateIoSizes(input_window_, playground);
    ApplyViewportFromIo(playground->io());

    playground->BeginFrame();
    cg::rhi::GetDevice().SetClearColor({cg::kClearColor.x * cg::kClearColor.w,
                                        cg::kClearColor.y * cg::kClearColor.w,
                                        cg::kClearColor.z * cg::kClearColor.w,
                                        cg::kClearColor.w});
    cg::rhi::GetDevice().Clear(cg::rhi::ClearMask::kDepth | cg::rhi::ClearMask::kColor);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    imgui_io_->FontGlobalScale = kImGuiScale;
    ImGui::NewFrame();

    FillIoInput(input_window_, imgui_io_, playground->mutable_io());

    const cg::Io io = playground->io();
    if (playground->io().gui_captured_cursor()) {
      glfwSetInputMode(input_window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
      glfwSetInputMode(input_window_, GLFW_CURSOR, io.left_button_pressed() ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }

    playground->Update();
    playground->Render();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    const glm::ivec2 framebuffer_size = playground->io().framebuffer_size();
    pixel_buffer_.resize(static_cast<std::size_t>(framebuffer_size.x) *
                         static_cast<std::size_t>(framebuffer_size.y) * 4);
    cg::rhi::GetDevice().SetReadBuffer(UsesSingleBufferedReadback() ? cg::rhi::ReadBuffer::kFront
                                                                    : cg::rhi::ReadBuffer::kBack);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    cg::rhi::GetDevice().ReadPixels({0, 0},
                                    framebuffer_size,
                                    cg::rhi::PixelFormat::kRGBA,
                                    cg::rhi::PixelType::kUInt8,
                                    pixel_buffer_.data());

    playground->EndFrame();

    return {
        .width = framebuffer_size.x,
        .height = framebuffer_size.y,
        .rgba_pixels = pixel_buffer_.data(),
        .size_in_bytes = pixel_buffer_.size(),
    };
  }

  const char* runtime_name() const {
    switch (offscreen_backend_) {
      case OffscreenBackend::kHiddenNativeOpenGL:
        return "Hidden Native OpenGL";
      case OffscreenBackend::kMesaOSMesa:
        return "Mesa OSMesa";
      case OffscreenBackend::kMesaEGL:
        return "Mesa EGL llvmpipe";
    }
    return "Unknown";
  }

  void MakeCurrentForCleanup() {
    MakeCurrent();
  }

 private:
  bool UsesSingleBufferedReadback() const {
    return offscreen_backend_ != OffscreenBackend::kHiddenNativeOpenGL;
  }

  void CreateOffscreenContext(int width, int height) {
    const OpenGLRuntime requested_gl_runtime = ChooseOpenGLRuntime();
    if (requested_gl_runtime == OpenGLRuntime::kMesa) {
      std::vector<std::string> mesa_failures;
#if defined(CG_HAS_MESA_EGL)
      std::string egl_error;
      if (TryCreateMesaEglContext(width, height, &egl_error)) {
        return;
      }
      if (!egl_error.empty()) {
        mesa_failures.push_back("Mesa EGL: " + egl_error);
      }
#endif
#if defined(CG_HAS_OSMESA)
      std::string osmesa_error;
      if (TryCreateOsMesaContext(width, height, &osmesa_error)) {
        return;
      }
      if (!osmesa_error.empty()) {
        mesa_failures.push_back("OSMesa: " + osmesa_error);
      }
#endif

      if (IsMesaOpenGLRuntimeExplicitlyRequested()) {
        if (mesa_failures.empty()) {
          CGLOG(ERROR) << "CG_GL_RUNTIME=mesa was requested, but this build does not have a Mesa offscreen runtime. "
                       << "Falling back to hidden native OpenGL.";
        } else {
          CGLOG(ERROR) << "CG_GL_RUNTIME=mesa was requested, but Mesa offscreen initialization failed ("
                       << JoinMessages(mesa_failures) << "). Falling back to hidden native OpenGL.";
        }
      }
    }

    CreateHiddenNativeContext(width, height);
  }

  void CreateHiddenNativeContext(int width, int height) {
    offscreen_backend_ = OffscreenBackend::kHiddenNativeOpenGL;
    glfwDefaultWindowHints();

#if defined(CG_PLATFORM_MACOS)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glsl_version_ = "#version 410";
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glsl_version_ = "#version 450";
#endif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    offscreen_window_ = glfwCreateWindow(width, height, "CG Offscreen GL", nullptr, nullptr);
    CGCHECK(offscreen_window_ != nullptr) << "Failed to create hidden offscreen OpenGL window.";
  }

#if defined(CG_HAS_OSMESA)
  bool TryCreateOsMesaContext(int width, int height, std::string* error) {
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_OSMESA_CONTEXT_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    offscreen_window_ = glfwCreateWindow(width, height, "CG Offscreen GL", nullptr, nullptr);
    if (offscreen_window_ == nullptr) {
      if (error != nullptr) {
        *error = "glfwCreateWindow returned null";
      }
      return false;
    }

    offscreen_backend_ = OffscreenBackend::kMesaOSMesa;
    glsl_version_ = "#version 450";
    if (error != nullptr) {
      error->clear();
    }
    return true;
  }
#endif

#if defined(CG_HAS_MESA_EGL)
  bool TryCreateMesaEglContext(int width, int height, std::string* error) {
    ConfigureMesaLlvmPipeEnvironment();

    using GetPlatformDisplayProc = EGLDisplay (*)(EGLenum, void*, const EGLAttrib*);
    auto get_platform_display =
        reinterpret_cast<GetPlatformDisplayProc>(eglGetProcAddress("eglGetPlatformDisplay"));
    if (get_platform_display == nullptr) {
      get_platform_display =
          reinterpret_cast<GetPlatformDisplayProc>(eglGetProcAddress("eglGetPlatformDisplayEXT"));
    }
    if (get_platform_display == nullptr) {
      if (error != nullptr) {
        *error = "eglGetPlatformDisplay{EXT} unavailable";
      }
      return false;
    }

    egl_display_ = get_platform_display(EGL_PLATFORM_SURFACELESS_MESA, nullptr, nullptr);
    if (egl_display_ == EGL_NO_DISPLAY) {
      if (error != nullptr) {
        *error = FormatEglErrorMessage("eglGetPlatformDisplay");
      }
      return false;
    }

    if (!eglInitialize(egl_display_, nullptr, nullptr)) {
      if (error != nullptr) {
        *error = FormatEglErrorMessage("eglInitialize");
      }
      DestroyMesaEglContext();
      return false;
    }

    const EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE,
    };
    EGLConfig config = nullptr;
    EGLint num_configs = 0;
    if (!eglChooseConfig(egl_display_, config_attribs, &config, 1, &num_configs) || num_configs < 1) {
      if (error != nullptr) {
        *error = FormatEglErrorMessage("eglChooseConfig");
      }
      DestroyMesaEglContext();
      return false;
    }

    if (!eglBindAPI(EGL_OPENGL_API)) {
      if (error != nullptr) {
        *error = FormatEglErrorMessage("eglBindAPI");
      }
      DestroyMesaEglContext();
      return false;
    }

    const EGLint context_attribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 4,
        EGL_CONTEXT_MINOR_VERSION, 5,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_NONE,
    };
    egl_context_ = eglCreateContext(egl_display_, config, EGL_NO_CONTEXT, context_attribs);
    if (egl_context_ == EGL_NO_CONTEXT) {
      if (error != nullptr) {
        *error = FormatEglErrorMessage("eglCreateContext");
      }
      DestroyMesaEglContext();
      return false;
    }

    const EGLint pbuffer_attribs[] = {
        EGL_WIDTH, width,
        EGL_HEIGHT, height,
        EGL_NONE,
    };
    egl_surface_ = eglCreatePbufferSurface(egl_display_, config, pbuffer_attribs);
    if (egl_surface_ == EGL_NO_SURFACE) {
      if (error != nullptr) {
        *error = FormatEglErrorMessage("eglCreatePbufferSurface");
      }
      DestroyMesaEglContext();
      return false;
    }

    if (!eglMakeCurrent(egl_display_, egl_surface_, egl_surface_, egl_context_)) {
      if (error != nullptr) {
        *error = FormatEglErrorMessage("eglMakeCurrent");
      }
      DestroyMesaEglContext();
      return false;
    }

    InitializeOpenGLLoader(reinterpret_cast<GLADloadfunc>(eglGetProcAddress));
    offscreen_backend_ = OffscreenBackend::kMesaEGL;
    glsl_version_ = "#version 450";
    if (error != nullptr) {
      error->clear();
    }
    return true;
  }

  void DestroyMesaEglContext() {
    if (egl_display_ != EGL_NO_DISPLAY) {
      eglMakeCurrent(egl_display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }
    if (egl_surface_ != EGL_NO_SURFACE) {
      eglDestroySurface(egl_display_, egl_surface_);
      egl_surface_ = EGL_NO_SURFACE;
    }
    if (egl_context_ != EGL_NO_CONTEXT) {
      eglDestroyContext(egl_display_, egl_context_);
      egl_context_ = EGL_NO_CONTEXT;
    }
    if (egl_display_ != EGL_NO_DISPLAY) {
      eglTerminate(egl_display_);
      egl_display_ = EGL_NO_DISPLAY;
    }
  }
#endif

  void MakeCurrent() {
    if (offscreen_backend_ == OffscreenBackend::kMesaEGL) {
#if defined(CG_HAS_MESA_EGL)
      CGCHECK(eglMakeCurrent(egl_display_, egl_surface_, egl_surface_, egl_context_));
      InitializeOpenGLLoader(reinterpret_cast<GLADloadfunc>(eglGetProcAddress));
      return;
#else
      CGCHECK(false) << "Mesa EGL runtime selected without CG_HAS_MESA_EGL support.";
#endif
    }

    glfwMakeContextCurrent(offscreen_window_);
    InitializeOpenGLLoader(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress));
  }

  GLFWwindow* input_window_ = nullptr;
  GLFWwindow* offscreen_window_ = nullptr;
  ImGuiIO* imgui_io_ = nullptr;
  OffscreenBackend offscreen_backend_ = OffscreenBackend::kHiddenNativeOpenGL;
  const char* glsl_version_ = "#version 450";
  std::vector<std::uint8_t> pixel_buffer_;
#if defined(CG_HAS_MESA_EGL)
  EGLDisplay egl_display_ = EGL_NO_DISPLAY;
  EGLContext egl_context_ = EGL_NO_CONTEXT;
  EGLSurface egl_surface_ = EGL_NO_SURFACE;
#endif
};

GLuint CompilePresenterShader(const char* debug_name, GLenum shader_type, const std::string& source) {
  const GLuint shader = glCreateShader_(shader_type);
  const char* source_ptr = source.c_str();
  glShaderSource_(shader, 1, &source_ptr, nullptr);
  glCompileShader_(shader);

  int compile_ok = 0;
  glGetShaderiv_(shader, GL_COMPILE_STATUS, &compile_ok);
  if (!compile_ok) {
    char info_log[2048] = {};
    glGetShaderInfoLog_(shader, sizeof(info_log), nullptr, info_log);
    CGCHECK(false) << debug_name << " compile failed: " << info_log;
  }
  return shader;
}

GLuint LinkPresenterProgram(GLuint vertex_shader, GLuint fragment_shader) {
  const GLuint program = glCreateProgram_();
  glAttachShader_(program, vertex_shader);
  glAttachShader_(program, fragment_shader);
  glLinkProgram_(program);

  int link_ok = 0;
  glGetProgramiv_(program, GL_LINK_STATUS, &link_ok);
  if (!link_ok) {
    char info_log[2048] = {};
    glGetProgramInfoLog_(program, sizeof(info_log), nullptr, info_log);
    CGCHECK(false) << "OpenGL presenter link failed: " << info_log;
  }
  return program;
}

class OpenGLPresenter {
 public:
  OpenGLPresenter(const WindowConfig& config, const char* title)
      : window_(CreateVisibleOpenGLWindow(config, title)) {
    MakeCurrent();
    glfwSwapInterval(1);

    const std::string vertex_source = std::string(PresenterGLSLVersion()) + R"(
      out vec2 uv;

      void main() {
        const vec2 positions[3] = vec2[3](
            vec2(-1.0, -1.0),
            vec2( 3.0, -1.0),
            vec2(-1.0,  3.0));
        const vec2 texcoords[3] = vec2[3](
            vec2(0.0, 0.0),
            vec2(2.0, 0.0),
            vec2(0.0, 2.0));
        gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);
        uv = vec2(texcoords[gl_VertexID].x, 1.0 - texcoords[gl_VertexID].y);
      }
    )";
    const std::string fragment_source = std::string(PresenterGLSLVersion()) + R"(
      in vec2 uv;
      out vec4 out_color;
      uniform sampler2D frame_texture;

      void main() {
        out_color = texture(frame_texture, uv);
      }
    )";

    const GLuint vertex_shader =
        CompilePresenterShader("OpenGL presenter vertex shader", GL_VERTEX_SHADER, vertex_source);
    const GLuint fragment_shader =
        CompilePresenterShader("OpenGL presenter fragment shader", GL_FRAGMENT_SHADER, fragment_source);
    program_ = LinkPresenterProgram(vertex_shader, fragment_shader);
    glDeleteShader_(vertex_shader);
    glDeleteShader_(fragment_shader);

    glGenVertexArrays_(1, &vao_);
    glGenTextures_(1, &texture_);
    glBindTexture_(GL_TEXTURE_2D, texture_);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture_(GL_TEXTURE_2D, 0);
  }

  ~OpenGLPresenter() {
    MakeCurrent();
    if (texture_ != 0) {
      glDeleteTextures_(1, &texture_);
    }
    if (vao_ != 0) {
      glDeleteVertexArrays_(1, &vao_);
    }
    if (program_ != 0) {
      glDeleteProgram_(program_);
    }
    if (window_ != nullptr) {
      glfwDestroyWindow(window_);
    }
  }

  GLFWwindow* window() const {
    return window_;
  }

  bool ShouldClose() const {
    return glfwWindowShouldClose(window_) != 0;
  }

  const char* runtime_name() const {
    return "OpenGL";
  }

  void Present(const OffscreenGlRenderer::Frame& frame) {
    MakeCurrent();

    if (frame.width != texture_width_ || frame.height != texture_height_) {
      texture_width_ = frame.width;
      texture_height_ = frame.height;
      glBindTexture_(GL_TEXTURE_2D, texture_);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glTexImage2D_(GL_TEXTURE_2D, 0, GL_RGBA8, texture_width_, texture_height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
      glBindTexture_(GL_TEXTURE_2D, 0);
    }

    glBindTexture_(GL_TEXTURE_2D, texture_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D_(GL_TEXTURE_2D,
                     0,
                     0,
                     0,
                     frame.width,
                     frame.height,
                     GL_RGBA,
                     GL_UNSIGNED_BYTE,
                     frame.rgba_pixels);

    int framebuffer_width = 0;
    int framebuffer_height = 0;
    glfwGetFramebufferSize(window_, &framebuffer_width, &framebuffer_height);
    glViewport_(0, 0, framebuffer_width, framebuffer_height);
    glDisable_(GL_DEPTH_TEST);
    glClearColor_(0.0f, 0.0f, 0.0f, 1.0f);
    glClear_(GL_COLOR_BUFFER_BIT);

    glUseProgram_(program_);
    glBindVertexArray_(vao_);
    glActiveTexture_(GL_TEXTURE0);
    glBindTexture_(GL_TEXTURE_2D, texture_);
    glUniform1i_(glGetUniformLocation_(program_, "frame_texture"), 0);
    glDrawArrays_(GL_TRIANGLES, 0, 3);
    glBindVertexArray_(0);

    glfwSwapBuffers(window_);
  }

 private:
  void MakeCurrent() {
    glfwMakeContextCurrent(window_);
    InitializeOpenGLLoader(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress));
  }

  GLFWwindow* window_ = nullptr;
  GLuint program_ = 0;
  GLuint vao_ = 0;
  GLuint texture_ = 0;
  int texture_width_ = 0;
  int texture_height_ = 0;
};

void InitializeLogging(char** argv) {
  FLAGS_log_dir = "log";
  FLAGS_timestamp_in_logfile_name = false;
  google::InitGoogleLogging(argv[0]);
  google::SetLogDestination(google::GLOG_INFO, "log/log.txt");
}

void FillClipboard(Playground* playground, GLFWwindow* window) {
  auto write_clipboard = [window](const std::string& content) {
    glfwSetClipboardString(window, content.c_str());
  };
  playground->mutable_io()->SetWriteClipboardFunc(write_clipboard);
}

int RunLegacyOpenGL(const WindowConfig& config) {
  if (IsMesaOpenGLRuntimeExplicitlyRequested() && !ShouldUseMesaOpenGLRuntime()) {
    CGLOG(ERROR) << "CG_GL_RUNTIME=mesa was requested, but this build does not have a Mesa offscreen runtime. "
                 << "Falling back to native OpenGL presentation + rendering.";
  }
  GLFWwindow* window = CreateVisibleOpenGLWindow(config, "CG");

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);
  InitializeOpenGLLoader(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress));

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  ImGuiIO& imgui_io = ImGui::GetIO();

  ImPlot::CreateContext();

  ImGui_ImplGlfw_InitForOpenGL(window, true);
#if defined(CG_PLATFORM_MACOS)
  ImGui_ImplOpenGL3_Init("#version 410");
#else
  ImGui_ImplOpenGL3_Init("#version 450");
#endif

  cg::rhi::InitializeOpenGLDevice();
  CGLOG(ERROR) << "RHI Runtime API : " << RuntimeRhiApiName();
  CGLOG(ERROR) << "Scene Renderer API : " << SceneRendererApiName();
  LogSceneCapabilitySummary();

  Playground playground;
  playground.SetPresentationRuntimeName("OpenGL");
  FillClipboard(&playground, window);
  UpdateIoSizes(window, &playground);
  ApplyViewportFromIo(playground.io());

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    UpdateIoSizes(window, &playground);
    ApplyViewportFromIo(playground.io());

    playground.BeginFrame();
    cg::rhi::GetDevice().SetClearColor({cg::kClearColor.x * cg::kClearColor.w,
                                        cg::kClearColor.y * cg::kClearColor.w,
                                        cg::kClearColor.z * cg::kClearColor.w,
                                        cg::kClearColor.w});
    cg::rhi::GetDevice().Clear(cg::rhi::ClearMask::kDepth | cg::rhi::ClearMask::kColor);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    imgui_io.FontGlobalScale = kImGuiScale;
    ImGui::NewFrame();

    FillIoInput(window, &imgui_io, playground.mutable_io());
    const cg::Io io = playground.io();
    if (playground.io().gui_captured_cursor()) {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
      glfwSetInputMode(window, GLFW_CURSOR, io.left_button_pressed() ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }

    playground.Update();
    playground.Render();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);

    playground.EndFrame();
  }

  glfwMakeContextCurrent(window);
  playground.Destoy();
  cg::rhi::SetDevice(nullptr);
  ImPlot::DestroyContext();
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(window);
  return 0;
}

int RunPresentedOpenGL(const WindowConfig& config) {
  OpenGLPresenter presenter(config, "CG");
  int framebuffer_width = 0;
  int framebuffer_height = 0;
  glfwGetFramebufferSize(presenter.window(), &framebuffer_width, &framebuffer_height);
  OffscreenGlRenderer renderer(presenter.window(), framebuffer_width, framebuffer_height, RuntimeMode::kOpenGL);
  Playground playground;
  playground.SetPresentationRuntimeName(presenter.runtime_name());
  FillClipboard(&playground, presenter.window());
  UpdateIoSizes(presenter.window(), &playground);

  CGLOG(ERROR) << "Presentation Runtime : " << presenter.runtime_name();

  while (!presenter.ShouldClose()) {
    glfwPollEvents();
    const OffscreenGlRenderer::Frame frame = renderer.RenderFrame(&playground);
    presenter.Present(frame);
  }

  renderer.MakeCurrentForCleanup();
  playground.Destoy();
  cg::rhi::SetDevice(nullptr);
  return 0;
}

#if defined(CG_HAS_VULKAN_RHI)
int RunVulkan(const WindowConfig& config) {
  cg::rhi::vulkan::Presenter presenter(config.width, config.height, "CG");
  int framebuffer_width = 0;
  int framebuffer_height = 0;
  glfwGetFramebufferSize(presenter.window(), &framebuffer_width, &framebuffer_height);
  OffscreenGlRenderer renderer(presenter.window(), framebuffer_width, framebuffer_height, RuntimeMode::kVulkan);
  Playground playground;
  playground.SetPresentationRuntimeName(presenter.runtime_name());
  FillClipboard(&playground, presenter.window());
  UpdateIoSizes(presenter.window(), &playground);

  CGLOG(ERROR) << "Presentation Runtime : " << presenter.runtime_name();

  while (!presenter.ShouldClose()) {
    glfwPollEvents();
    const OffscreenGlRenderer::Frame frame = renderer.RenderFrame(&playground);
    presenter.Present({
        .width = frame.width,
        .height = frame.height,
        .rgba_pixels = frame.rgba_pixels,
        .size_in_bytes = frame.size_in_bytes,
    });
  }

  renderer.MakeCurrentForCleanup();
  playground.Destoy();
  cg::rhi::SetDevice(nullptr);
  return 0;
}
#endif

}  // namespace

int main(int argc, char** argv) {
  InitializeLogging(argv);

  glfwSetErrorCallback(glfw_error_callback);
  CGCHECK(glfwInit()) << "glfw Init Failed";

  const WindowConfig window_config = BuildWindowConfig();
  const RuntimeMode runtime_mode = ChooseRuntimeMode();

  int exit_code = 0;
  switch (runtime_mode) {
    case RuntimeMode::kOpenGL:
      exit_code = ShouldUseMesaOpenGLRuntime() ? RunPresentedOpenGL(window_config) : RunLegacyOpenGL(window_config);
      break;
#if defined(CG_HAS_VULKAN_RHI)
    case RuntimeMode::kVulkan:
      exit_code = RunVulkan(window_config);
      break;
#endif
  }

  glfwTerminate();
  google::ShutdownGoogleLogging();
  return exit_code;
}
