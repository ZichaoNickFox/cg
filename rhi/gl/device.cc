#include "rhi/device.h"

#include <cstdlib>
#include <unordered_map>
#include <vector>

#include <glm/gtc/type_ptr.hpp>

#include "base/util.h"
#include "renderer/gl.h"
#include "rhi/gl/types.h"
#include "renderer/texture.h"

namespace cg::rhi {
namespace {

GLenum ToGLBufferTarget(BufferType type) {
  switch (type) {
    case BufferType::kVertex:
      return GL_ARRAY_BUFFER;
    case BufferType::kIndex:
      return GL_ELEMENT_ARRAY_BUFFER;
    case BufferType::kStorage:
      return GL_SHADER_STORAGE_BUFFER;
    case BufferType::kAtomicCounter:
      return GL_ATOMIC_COUNTER_BUFFER;
    case BufferType::kTexture:
      return GL_TEXTURE_BUFFER;
  }
  CGCHECK(false) << "Unsupported buffer type";
  return 0;
}

GLenum ToGLBufferUsage(BufferUsage usage) {
  switch (usage) {
    case BufferUsage::kStatic:
      return GL_STATIC_DRAW;
    case BufferUsage::kDynamic:
      return GL_DYNAMIC_DRAW;
    case BufferUsage::kStreamCopy:
      return GL_STREAM_COPY;
  }
  CGCHECK(false) << "Unsupported buffer usage";
  return 0;
}

GLenum ToGLMapAccess(MapAccess access) {
  switch (access) {
    case MapAccess::kReadOnly:
      return GL_READ_ONLY;
  }
  CGCHECK(false) << "Unsupported map access";
  return 0;
}

GLenum ToGLTextureAccess(TextureAccess access) {
  switch (access) {
    case TextureAccess::kReadOnly:
      return GL_READ_ONLY;
    case TextureAccess::kWriteOnly:
      return GL_WRITE_ONLY;
    case TextureAccess::kReadWrite:
      return GL_READ_WRITE;
  }
  CGCHECK(false) << "Unsupported texture access";
  return 0;
}

GLbitfield ToGLMemoryBarrier(MemoryBarrier barrier) {
  switch (barrier) {
    case MemoryBarrier::kAll:
      return GL_ALL_BARRIER_BITS;
  }
  CGCHECK(false) << "Unsupported memory barrier";
  return 0;
}

GLenum ToGLFramebufferBindPoint(FramebufferBindPoint bind_point) {
  switch (bind_point) {
    case FramebufferBindPoint::kAll:
      return GL_FRAMEBUFFER;
    case FramebufferBindPoint::kRead:
      return GL_READ_FRAMEBUFFER;
    case FramebufferBindPoint::kDraw:
      return GL_DRAW_FRAMEBUFFER;
  }
  CGCHECK(false) << "Unsupported framebuffer bind point";
  return 0;
}

GLenum ToGLAttachment(AttachmentType attachment_type, uint32_t attachment_index) {
  switch (attachment_type) {
    case AttachmentType::kColor:
      return GL_COLOR_ATTACHMENT0 + attachment_index;
    case AttachmentType::kDepth:
      return GL_DEPTH_ATTACHMENT;
    case AttachmentType::kStencil:
      return GL_STENCIL_ATTACHMENT;
  }
  CGCHECK(false) << "Unsupported attachment type";
  return 0;
}

GLenum ToGLReadBuffer(ReadBuffer buffer) {
  switch (buffer) {
    case ReadBuffer::kFront:
      return GL_FRONT;
    case ReadBuffer::kBack:
      return GL_BACK;
  }
  CGCHECK(false) << "Unsupported read buffer";
  return 0;
}

GLenum ToGLBlitFilter(FilterMode filter) {
  switch (filter) {
    case FilterMode::kNearest:
      return GL_NEAREST;
    case FilterMode::kLinear:
      return GL_LINEAR;
    case FilterMode::kLinearMipmapLinear:
      break;
  }
  CGCHECK(false) << "Unsupported blit filter";
  return 0;
}

GLenum ToGLTextureTarget(Texture::Type type) {
  switch (type) {
    case Texture::kTexture2D:
      return GL_TEXTURE_2D;
    case Texture::kCubemap:
      return GL_TEXTURE_CUBE_MAP;
    case Texture::kTexture2DArray:
      return GL_TEXTURE_2D_ARRAY;
    case Texture::kUnknown:
      break;
  }
  CGCHECK(false) << "Unsupported texture target";
  return 0;
}

std::string GetOneLineCompileError430(const std::string& shader_name,
                                      const std::vector<ShaderCodePart>& code_parts,
                                      const std::string& gl_log) {
  std::string res = gl_log;
  const std::string starting = "0(";
  res.erase(0, starting.size());

  int line_num_end = res.find_first_of(")");
  int source_line_num = std::atoi(res.substr(0, line_num_end).c_str());
  res.erase(0, line_num_end);

  const std::string next = ") ";
  res.erase(0, next.size());

  int line_sum = 1;
  for (const ShaderCodePart& code_part : code_parts) {
    int line_num_in_file = 1;
    for (char c : code_part.code) {
      if (c == '\n') {
        ++line_sum;
        ++line_num_in_file;
        if (line_sum == source_line_num) {
          return util::Format("\"{}\" {} {}", code_part.glsl_path, line_num_in_file, res);
        }
      }
    }
  }
  CGKILL("Cannot find a file? source_line_num ~ ") << shader_name << " | " << gl_log;
  return "";
}

std::string GetMultipleLineCompileError430(const std::string& shader_name,
                                           const std::vector<ShaderCodePart>& code_parts,
                                           const std::string& gl_log) {
  std::vector<std::string> error_logs;
  std::string one_error;
  for (const char c : gl_log) {
    if (c == '\n') {
      error_logs.push_back(one_error);
      one_error.clear();
    } else {
      one_error += c;
    }
  }
  if (!one_error.empty()) {
    error_logs.push_back(one_error);
  }

  std::string res = "\n";
  for (const std::string& error_log : error_logs) {
    res += GetOneLineCompileError430(shader_name, code_parts, error_log);
    res += "\n";
  }
  return res;
}

uint32_t CompileShaderObject(const std::string& shader_name,
                             const std::vector<ShaderCodePart>& code_parts,
                             uint32_t shader_type) {
  GLuint object = glCreateShader_(shader_type);

  std::vector<const char*> code_data(code_parts.size());
  for (size_t i = 0; i < code_parts.size(); ++i) {
    code_data[i] = code_parts[i].code.data();
  }

  glShaderSource_(object, code_data.size(), code_data.data(), nullptr);
  glCompileShader_(object);

  int success = 0;
  char info_log[1024];
  glGetShaderiv_(object, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog_(object, 1024, nullptr, info_log);
    std::string compile_error_info(info_log);
    CGKILL("Shader Compile Error : name~")
        << shader_name << GetMultipleLineCompileError430(shader_name, code_parts, compile_error_info);
  }

  return object;
}

void LinkProgramObject(const std::string& shader_name, uint32_t program, const std::vector<uint32_t>& objects) {
  for (uint32_t object : objects) {
    glAttachShader_(program, object);
  }
  glLinkProgram_(program);

  int success = 0;
  char info_log[1024];
  glGetProgramiv_(program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog_(program, 1024, nullptr, info_log);
    CGCHECK(false) << shader_name << " : Program link error :" << info_log;
  }

  for (uint32_t object : objects) {
    glDeleteShader_(object);
  }
}

Capabilities DetectOpenGLCapabilities() {
  GLint major = 0;
  GLint minor = 0;
  glGetIntegerv_(GL_MAJOR_VERSION, &major);
  glGetIntegerv_(GL_MINOR_VERSION, &minor);

  const bool supports_opengl_45 = (major > 4) || (major == 4 && minor >= 5);
  const GLubyte* version_string = glGetString(GL_VERSION);
  const GLubyte* shader_language_string = glGetString(GL_SHADING_LANGUAGE_VERSION);

  Capabilities caps;
  caps.backend = BackendType::kOpenGL;
  caps.scene_backend = BackendType::kOpenGL;
  caps.graphics_api_name = version_string == nullptr ? "" : reinterpret_cast<const char*>(version_string);
  caps.scene_api_name = caps.graphics_api_name;
  caps.shader_language_name =
      shader_language_string == nullptr ? "" : reinterpret_cast<const char*>(shader_language_string);
  caps.supports_glsl_450 = supports_opengl_45;
  caps.supports_storage_buffers = supports_opengl_45;
  caps.supports_storage_images = supports_opengl_45;
  caps.supports_compute = supports_opengl_45;
  return caps;
}

class OpenGLBuffer final : public Buffer {
 public:
  explicit OpenGLBuffer(BufferType type) : type_(type) {
    glGenBuffers_(1, &id_);
  }

  ~OpenGLBuffer() override {
    glDeleteBuffers_(1, &id_);
  }

  BufferType type() const override {
    return type_;
  }

  void SetData(size_t size_in_bytes, const void* data, BufferUsage usage) override {
    const GLenum target = ToGLBufferTarget(type_);
    glBindBuffer_(target, id_);
    glBufferData_(target, size_in_bytes, data, ToGLBufferUsage(usage));
    glBindBuffer_(target, 0);
  }

  void UpdateData(size_t offset_in_bytes, size_t size_in_bytes, const void* data) override {
    const GLenum target = ToGLBufferTarget(type_);
    glBindBuffer_(target, id_);
    glBufferSubData_(target, offset_in_bytes, size_in_bytes, data);
    glBindBuffer_(target, 0);
  }

  void Bind() override {
    glBindBuffer_(ToGLBufferTarget(type_), id_);
  }

  void Unbind() override {
    glBindBuffer_(ToGLBufferTarget(type_), 0);
  }

  void BindBase(uint32_t binding_point) override {
    glBindBufferBase_(ToGLBufferTarget(type_), binding_point, id_);
  }

  void* Map(MapAccess access) override {
    const GLenum target = ToGLBufferTarget(type_);
    glBindBuffer_(target, id_);
    return glMapBuffer_(target, ToGLMapAccess(access));
  }

  void Unmap() override {
    const GLenum target = ToGLBufferTarget(type_);
    glUnmapBuffer_(target);
    glBindBuffer_(target, 0);
  }

 private:
  BufferType type_;
  GLuint id_ = 0;
};

class OpenGLVertexArray final : public VertexArray {
 public:
  OpenGLVertexArray() {
    glGenVertexArrays_(1, &id_);
  }

  ~OpenGLVertexArray() override {
    if (id_ != 0) {
      glDeleteVertexArrays_(1, &id_);
    }
  }

  void Bind() override {
    glBindVertexArray_(id_);
  }

  void Unbind() override {
    glBindVertexArray_(0);
  }

  void EnableAttribute(uint32_t index) override {
    glEnableVertexAttribArray_(index);
  }

  void SetFloatAttribute(uint32_t index,
                         int component_count,
                         int stride_in_bytes,
                         size_t offset_in_bytes) override {
    glVertexAttribPointer_(index,
                           component_count,
                           GL_FLOAT,
                           GL_FALSE,
                           stride_in_bytes,
                           reinterpret_cast<const void*>(offset_in_bytes));
  }

  void SetAttributeDivisor(uint32_t index, uint32_t divisor) override {
    glVertexAttribDivisor_(index, divisor);
  }

 private:
  GLuint id_ = 0;
};

class OpenGLProgram final : public Program {
 public:
  OpenGLProgram(const std::string& name,
                const std::vector<ShaderCodePart>& vs,
                const std::vector<ShaderCodePart>& fs,
                const std::vector<ShaderCodePart>& gs,
                const std::vector<ShaderCodePart>& ts)
      : name_(name) {
    const bool has_gs = !gs.empty();
    const bool has_ts = !ts.empty();
    const GLuint vertex_shader_object = CompileShaderObject(name_, vs, GL_VERTEX_SHADER);
    const GLuint fragment_shader_object = CompileShaderObject(name_, fs, GL_FRAGMENT_SHADER);
    GLuint geometry_shader_object = 0;
    GLuint tessellation_shader_object = 0;
    if (has_gs) {
      geometry_shader_object = CompileShaderObject(name_, gs, GL_GEOMETRY_SHADER);
    }
    if (has_ts) {
      tessellation_shader_object = CompileShaderObject(name_, ts, GL_TESS_CONTROL_SHADER);
    }

    id_ = glCreateProgram_();
    std::vector<uint32_t> objects{vertex_shader_object, fragment_shader_object};
    if (has_gs) {
      objects.push_back(geometry_shader_object);
    }
    if (has_ts) {
      objects.push_back(tessellation_shader_object);
    }
    LinkProgramObject(name_, id_, objects);
  }

  OpenGLProgram(const std::string& name, const std::vector<ShaderCodePart>& cs)
      : name_(name) {
    id_ = glCreateProgram_();
    const GLuint compute_shader_object = CompileShaderObject(name_, cs, GL_COMPUTE_SHADER);
    LinkProgramObject(name_, id_, {compute_shader_object});
  }

  ~OpenGLProgram() override {
    if (id_ != 0) {
      glDeleteProgram_(id_);
    }
  }

  uint32_t id() const override {
    return id_;
  }

  void Use() const override {
    CGCHECK(glIsProgram_(id_)) << "glIsProgram failed, glCreateProgram? not glDeleteProgram? id ~ " << id_;
    glUseProgram_(id_);
    texture_2_unit_.clear();
  }

  void SetBool(const std::string& location_name, bool value) const override {
    glUniform1i_(GetUniformLocation(location_name), static_cast<int>(value));
  }

  void SetFloat(const std::string& location_name, float value) const override {
    glUniform1f_(GetUniformLocation(location_name), value);
  }

  void SetInt(const std::string& location_name, int value) const override {
    glUniform1i_(GetUniformLocation(location_name), value);
  }

  int BindTexture(const std::string& location_name, const Texture& value) const override {
    CGCHECK(!value.empty()) << location_name;
    int unit = -1;
    if (texture_2_unit_.count(value.id()) > 0) {
      unit = texture_2_unit_.at(value.id());
    } else {
      unit = texture_2_unit_.size();
      CGCHECK(unit <= 31) << "Only TEXTURE0 - TEXTURE31 supported : " << unit;
      texture_2_unit_[value.id()] = unit;
    }
    SetInt(location_name, unit);
    glActiveTexture_(GL_TEXTURE0 + unit);
    glBindTexture_(ToGLTextureTarget(value.meta().type), value.id());
    return unit;
  }

  void SetMat4(const std::string& location_name, const glm::mat4& value) const override {
    glUniformMatrix4fv_(GetUniformLocation(location_name), 1, GL_FALSE, glm::value_ptr(value));
  }

  void SetVec4(const std::string& location_name, const glm::vec4& value) const override {
    glUniform4fv_(GetUniformLocation(location_name), 1, glm::value_ptr(value));
  }

  void SetVec3(const std::string& location_name, const glm::vec3& value) const override {
    glUniform3fv_(GetUniformLocation(location_name), 1, glm::value_ptr(value));
  }

  void SetVec2(const std::string& location_name, const glm::vec2& value) const override {
    glUniform2fv_(GetUniformLocation(location_name), 1, glm::value_ptr(value));
  }

 private:
  int32_t GetUniformLocation(const std::string& location_name) const {
    int32_t res = glGetUniformLocation_(id_, location_name.c_str());
    if (res == GL_INVALID_VALUE || res == GL_INVALID_OPERATION) {
      CGKILL("GetUniformLocation Failed") << res;
    }
    return res;
  }

  std::string name_;
  uint32_t id_ = 0;
  mutable std::unordered_map<uint32_t, int> texture_2_unit_;
};

class OpenGLDevice final : public Device {
 public:
  OpenGLDevice() : capabilities_(DetectOpenGLCapabilities()) {}

  const Capabilities& capabilities() const override {
    return capabilities_;
  }

  std::unique_ptr<Buffer> CreateBuffer(BufferType type) override {
    return std::make_unique<OpenGLBuffer>(type);
  }

  std::unique_ptr<VertexArray> CreateVertexArray() override {
    return std::make_unique<OpenGLVertexArray>();
  }

  std::shared_ptr<Program> CreateRenderProgram(const std::string& name,
                                               const std::vector<ShaderCodePart>& vs,
                                               const std::vector<ShaderCodePart>& fs,
                                               const std::vector<ShaderCodePart>& gs,
                                               const std::vector<ShaderCodePart>& ts) override {
    return std::make_shared<OpenGLProgram>(name, vs, fs, gs, ts);
  }

  std::shared_ptr<Program> CreateComputeProgram(const std::string& name,
                                                const std::vector<ShaderCodePart>& cs) override {
    return std::make_shared<OpenGLProgram>(name, cs);
  }

  void EnsureTextureUploaded(Texture* texture) override {
    std::shared_ptr<Texture::Storage> storage = texture->storage();
    CGCHECK(storage != nullptr) << "Texture storage is unavailable.";
    if (storage->uploaded_to_gl) {
      return;
    }

    const Texture::Meta& meta = texture->meta();
    switch (meta.type) {
      case Texture::kTexture2D: {
        CGCHECK(storage->cpu_levels.size() == static_cast<size_t>(meta.level_num))
            << "Texture2D is missing mip payloads.";
        const GLenum target = ToGLTextureTarget(meta.type);
        glGenTextures_(1, &storage->id);
        glBindTexture_(target, storage->id);
        glTexStorage2D_(target, meta.level_num, ToGLTextureFormat(meta.format), meta.width, meta.height);
        for (int level = 0; level < meta.level_num; ++level) {
          glTexSubImage2D_(target,
                           level,
                           0,
                           0,
                           std::max(1, meta.width >> level),
                           std::max(1, meta.height >> level),
                           ToGLPixelFormat(meta.pixel_format),
                           ToGLPixelType(meta.pixel_type),
                           storage->cpu_levels[level].data());
        }
        glTexParameteri_(target, GL_TEXTURE_WRAP_S, ToGLWrapMode(meta.wrap_s));
        glTexParameteri_(target, GL_TEXTURE_WRAP_T, ToGLWrapMode(meta.wrap_t));
        if (meta.level_num > 1) {
          glTexParameteri_(target, GL_TEXTURE_MIN_FILTER, ToGLFilterMode(FilterMode::kLinearMipmapLinear));
        } else {
          glTexParameteri_(target, GL_TEXTURE_MIN_FILTER, ToGLFilterMode(meta.min_filter));
        }
        glTexParameteri_(target, GL_TEXTURE_MAG_FILTER, ToGLFilterMode(meta.mag_filter));
        glBindTexture_(target, 0);
        storage->uploaded_to_gl = true;
        return;
      }
      case Texture::kTexture2DArray: {
        CGCHECK(storage->cpu_levels.size() == 1) << "Texture2DArray expects a single packed CPU payload.";
        const GLenum target = ToGLTextureTarget(meta.type);
        glGenTextures_(1, &storage->id);
        glBindTexture_(target, storage->id);
        glTexImage3D_(target,
                      0,
                      ToGLTextureFormat(meta.format),
                      meta.width,
                      meta.height,
                      meta.depth,
                      0,
                      ToGLPixelFormat(meta.pixel_format),
                      ToGLPixelType(meta.pixel_type),
                      storage->cpu_levels[0].data());
        glTexParameteri_(target, GL_TEXTURE_MAG_FILTER, ToGLFilterMode(meta.mag_filter));
        glTexParameteri_(target, GL_TEXTURE_MIN_FILTER, ToGLFilterMode(meta.min_filter));
        glTexParameteri_(target, GL_TEXTURE_WRAP_S, ToGLWrapMode(meta.wrap_s));
        glTexParameteri_(target, GL_TEXTURE_WRAP_T, ToGLWrapMode(meta.wrap_t));
        glBindTexture_(target, 0);
        storage->uploaded_to_gl = true;
        return;
      }
      case Texture::kCubemap: {
        CGCHECK(storage->cpu_levels.size() == static_cast<size_t>(meta.level_num * 6))
            << "Cubemap is missing face payloads.";
        glGenTextures_(1, &storage->id);
        glBindTexture_(GL_TEXTURE_CUBE_MAP, storage->id);
        glTexStorage2D_(GL_TEXTURE_CUBE_MAP, meta.level_num, ToGLTextureFormat(meta.format), meta.width, meta.height);
        for (int level = 0; level < meta.level_num; ++level) {
          for (int face = 0; face < 6; ++face) {
            const std::vector<uint8_t>& face_level = storage->cpu_levels[static_cast<size_t>(level * 6 + face)];
            glTexSubImage2D_(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
                             level,
                             0,
                             0,
                             std::max(1, meta.width >> level),
                             std::max(1, meta.height >> level),
                             ToGLPixelFormat(meta.pixel_format),
                             ToGLPixelType(meta.pixel_type),
                             face_level.data());
          }
        }
        glTexParameteri_(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, ToGLWrapMode(meta.wrap_s));
        glTexParameteri_(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, ToGLWrapMode(meta.wrap_t));
        glTexParameteri_(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, ToGLWrapMode(WrapMode::kClampToEdge));
        glTexParameteri_(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, ToGLFilterMode(meta.min_filter));
        glTexParameteri_(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, ToGLFilterMode(meta.mag_filter));
        glBindTexture_(GL_TEXTURE_CUBE_MAP, 0);
        storage->uploaded_to_gl = true;
        return;
      }
      case Texture::kUnknown:
        break;
    }
    CGCHECK(false) << "Unsupported texture type for upload.";
  }

  void ReleaseTexture(Texture* texture) override {
    std::shared_ptr<Texture::Storage> storage = texture->storage();
    if (storage == nullptr || !storage->owns_gl_texture || storage->id == std::numeric_limits<uint32_t>::max()) {
      return;
    }
    const uint32_t texture_id = storage->id;
    glDeleteTextures_(1, &texture_id);
    storage->id = std::numeric_limits<uint32_t>::max();
    storage->uploaded_to_gl = false;
  }

  void ReadTextureData(const Texture& texture, int level, void* data, size_t size_in_bytes) override {
    const GLenum target = ToGLTextureTarget(texture.meta().type);
    glBindTexture_(target, texture.id());
    glGetTexImage_(target,
                   level,
                   ToGLPixelFormat(texture.meta().pixel_format),
                   ToGLPixelType(texture.meta().pixel_type),
                   data);
    glBindTexture_(target, 0);
  }

  void BindStorageTexture(uint32_t texture_unit, const Texture& texture, TextureAccess access) override {
    glBindImageTexture_(texture_unit, texture.id(), 0, GL_FALSE, 0, ToGLTextureAccess(access),
                        ToGLTextureFormat(texture.meta().format));
  }

  void DispatchCompute(const glm::uvec3& workgroup_count) override {
    glDispatchCompute_(workgroup_count.x, workgroup_count.y, workgroup_count.z);
  }

  void MemoryBarrier(rhi::MemoryBarrier barrier) override {
    glMemoryBarrier_(ToGLMemoryBarrier(barrier));
  }

  uint32_t CreateFramebuffer() override {
    uint32_t framebuffer = 0;
    glGenFramebuffers_(1, &framebuffer);
    return framebuffer;
  }

  void DeleteFramebuffer(uint32_t framebuffer) override {
    glDeleteFramebuffers_(1, &framebuffer);
  }

  void BindFramebuffer(FramebufferBindPoint bind_point, uint32_t framebuffer) override {
    glBindFramebuffer_(ToGLFramebufferBindPoint(bind_point), framebuffer);
  }

  void AttachFramebufferTexture2D(AttachmentType attachment_type,
                                  uint32_t attachment_index,
                                  const Texture& texture) override {
    glFramebufferTexture2D_(GL_FRAMEBUFFER, ToGLAttachment(attachment_type, attachment_index),
                            GL_TEXTURE_2D, texture.id(), 0);
  }

  bool CheckFramebufferComplete() override {
    return glCheckFramebufferStatus_(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
  }

  FramebufferState CaptureFramebufferState() const override {
    FramebufferState state;
    GLint viewport[4] = {};
    glGetIntegerv_(GL_VIEWPORT, viewport);
    glGetIntegerv_(GL_FRAMEBUFFER_BINDING, &state.framebuffer);
    state.viewport = glm::ivec4(viewport[0], viewport[1], viewport[2], viewport[3]);
    return state;
  }

  void RestoreFramebufferState(const FramebufferState& state) override {
    BindFramebuffer(FramebufferBindPoint::kAll, state.framebuffer);
    SetViewport({state.viewport.x, state.viewport.y}, {state.viewport.z, state.viewport.w});
  }

  void ClearColorAttachment(uint32_t attachment_index, const glm::vec4& color) override {
    glClearBufferfv_(GL_COLOR, attachment_index, glm::value_ptr(color));
  }

  void SetDrawBuffers(const std::vector<uint32_t>& color_attachment_indices) override {
    std::vector<GLenum> draw_buffers(color_attachment_indices.size());
    for (size_t i = 0; i < color_attachment_indices.size(); ++i) {
      draw_buffers[i] = ToGLAttachment(AttachmentType::kColor, color_attachment_indices[i]);
    }
    glDrawBuffers_(draw_buffers.size(), draw_buffers.data());
  }

  void BlitFramebuffer(uint32_t read_framebuffer,
                       uint32_t draw_framebuffer,
                       const glm::ivec2& size,
                       ClearMask mask,
                       FilterMode filter) override {
    BindFramebuffer(FramebufferBindPoint::kRead, read_framebuffer);
    BindFramebuffer(FramebufferBindPoint::kDraw, draw_framebuffer);
    glBlitFramebuffer_(0, 0, size.x, size.y, 0, 0, size.x, size.y, ToGLClearMask(mask), ToGLBlitFilter(filter));
  }

  void DrawArrays(PrimitiveTopology topology,
                  uint32_t first,
                  uint32_t count,
                  uint32_t instance_count) override {
    if (instance_count > 1) {
      glDrawArraysInstanced_(ToGLPrimitiveTopology(topology), first, count, instance_count);
    } else {
      glDrawArrays_(ToGLPrimitiveTopology(topology), first, count);
    }
  }

  void DrawElements(PrimitiveTopology topology,
                    uint32_t count,
                    uint32_t instance_count) override {
    if (instance_count > 1) {
      glDrawElementsInstanced_(ToGLPrimitiveTopology(topology), count, GL_UNSIGNED_INT, nullptr, instance_count);
    } else {
      glDrawElements_(ToGLPrimitiveTopology(topology), count, GL_UNSIGNED_INT, nullptr);
    }
  }

  void SetReadBuffer(ReadBuffer buffer) override {
    glReadBuffer_(ToGLReadBuffer(buffer));
  }

  void ReadPixels(const glm::ivec2& origin,
                  const glm::ivec2& size,
                  PixelFormat format,
                  PixelType type,
                  void* data) override {
    glReadPixels_(origin.x, origin.y, size.x, size.y, ToGLPixelFormat(format), ToGLPixelType(type), data);
  }

  void SetViewport(const glm::ivec2& origin, const glm::ivec2& size) override {
    glViewport_(origin.x, origin.y, size.x, size.y);
  }

  void SetClearColor(const glm::vec4& color) override {
    glClearColor_(color.r, color.g, color.b, color.a);
  }

  void Clear(ClearMask mask) override {
    glClear_(ToGLClearMask(mask));
  }

  void SetDepthTestEnabled(bool enabled) override {
    if (enabled) {
      glEnable_(GL_DEPTH_TEST);
    } else {
      glDisable_(GL_DEPTH_TEST);
    }
  }

  void SetCullEnabled(bool enabled) override {
    if (enabled) {
      glEnable_(GL_CULL_FACE);
    } else {
      glDisable_(GL_CULL_FACE);
    }
  }

  void SetCullMode(CullMode mode) override {
    glCullFace_(ToGLCullMode(mode));
  }

  void SetFrontFace(FrontFace winding) override {
    glFrontFace_(ToGLFrontFace(winding));
  }

 private:
  Capabilities capabilities_;
};

}  // namespace

std::unique_ptr<Device> CreateOpenGLDevice() {
  return std::make_unique<OpenGLDevice>();
}

void InitializeOpenGLDevice() {
  SetDevice(CreateOpenGLDevice());
}

}  // namespace cg::rhi
