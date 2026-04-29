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

template <class... Ts>
struct Overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

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

uint64_t AttachmentKey(AttachmentType attachment_type, uint32_t attachment_index) {
  return (static_cast<uint64_t>(attachment_index) << 32) | static_cast<uint32_t>(attachment_type);
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

  void BindBase(uint32_t binding_point) override {
    glBindBufferBase_(ToGLBufferTarget(type_), binding_point, id_);
  }

  void ReadData(const BufferReadDesc& desc, void* data) override {
    const GLenum target = ToGLBufferTarget(type_);
    glBindBuffer_(target, id_);
    glGetBufferSubData_(target, desc.offset_in_bytes, desc.size_in_bytes, data);
    glBindBuffer_(target, 0);
  }

  GLuint id() const {
    return id_;
  }

  GLenum target() const {
    return ToGLBufferTarget(type_);
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

  void ApplyBinding(const VertexArrayBindingDesc& desc) override {
    glBindVertexArray_(id_);
    if (desc.buffer != nullptr) {
      auto* gl_buffer = dynamic_cast<OpenGLBuffer*>(desc.buffer);
      CGCHECK(gl_buffer != nullptr) << "OpenGLVertexArray expected OpenGLBuffer for attribute binding.";
      glBindBuffer_(gl_buffer->target(), gl_buffer->id());
    }
    for (const VertexAttributeDesc& attribute : desc.attributes) {
      EnableAttribute(attribute.index);
      SetFloatAttribute(attribute.index,
                        attribute.component_count,
                        attribute.stride_in_bytes,
                        attribute.offset_in_bytes);
      if (attribute.divisor > 0) {
        SetAttributeDivisor(attribute.index, attribute.divisor);
      }
    }
    if (desc.buffer != nullptr) {
      auto* gl_buffer = dynamic_cast<OpenGLBuffer*>(desc.buffer);
      CGCHECK(gl_buffer != nullptr);
      glBindBuffer_(gl_buffer->target(), 0);
    }
    glBindVertexArray_(0);
  }

  GLuint id() const {
    return id_;
  }

 private:
  void EnableAttribute(uint32_t index) {
    glEnableVertexAttribArray_(index);
  }

  void SetFloatAttribute(uint32_t index,
                         int component_count,
                         int stride_in_bytes,
                         size_t offset_in_bytes) {
    glVertexAttribPointer_(index,
                           component_count,
                           GL_FLOAT,
                           GL_FALSE,
                           stride_in_bytes,
                           reinterpret_cast<const void*>(offset_in_bytes));
  }

  void SetAttributeDivisor(uint32_t index, uint32_t divisor) {
    glVertexAttribDivisor_(index, divisor);
  }
  GLuint id_ = 0;
};

class OpenGLProgram final : public Program {
 public:
  using BufferBindingHandler = std::function<void(const std::vector<BufferBindingDesc>& bindings)>;

  OpenGLProgram(const std::string& name,
                const std::vector<ShaderCodePart>& vs,
                const std::vector<ShaderCodePart>& fs,
                const std::vector<ShaderCodePart>& gs,
                const std::vector<ShaderCodePart>& ts,
                BufferBindingHandler buffer_binding_handler = {})
      : name_(name), buffer_binding_handler_(std::move(buffer_binding_handler)) {
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

  OpenGLProgram(const std::string& name,
                const std::vector<ShaderCodePart>& cs,
                BufferBindingHandler buffer_binding_handler = {})
      : name_(name), buffer_binding_handler_(std::move(buffer_binding_handler)) {
    id_ = glCreateProgram_();
    const GLuint compute_shader_object = CompileShaderObject(name_, cs, GL_COMPUTE_SHADER);
    LinkProgramObject(name_, id_, {compute_shader_object});
  }

  ~OpenGLProgram() override {
    if (id_ != 0) {
      glDeleteProgram_(id_);
    }
  }

  void ApplyBindings(const ProgramBindings& bindings) const override {
    Activate();
    for (const ProgramUniformBindingDesc& uniform : bindings.uniforms) {
      std::visit(
          Overloaded{
              [&](bool value) { SetBool(uniform.name, value); },
              [&](float value) { SetFloat(uniform.name, value); },
              [&](int value) { SetInt(uniform.name, value); },
              [&](const glm::mat4& value) { SetMat4(uniform.name, value); },
              [&](const glm::vec4& value) { SetVec4(uniform.name, value); },
              [&](const glm::vec3& value) { SetVec3(uniform.name, value); },
              [&](const glm::vec2& value) { SetVec2(uniform.name, value); },
          },
          uniform.value);
    }
    for (const ProgramTextureBindingDesc& texture_binding : bindings.textures) {
      CGCHECK(texture_binding.texture != nullptr) << texture_binding.name;
      BindTexture(texture_binding.name, *texture_binding.texture);
    }
    for (const ProgramStorageTextureBindingDesc& storage_texture_binding : bindings.storage_textures) {
      CGCHECK(storage_texture_binding.texture != nullptr) << storage_texture_binding.name;
      const int texture_unit = BindTexture(storage_texture_binding.name, *storage_texture_binding.texture);
      glBindImageTexture_(texture_unit,
                          storage_texture_binding.texture->id(),
                          0,
                          GL_FALSE,
                          0,
                          ToGLTextureAccess(storage_texture_binding.access),
                          ToGLTextureFormat(storage_texture_binding.texture->meta().format));
    }
    if (buffer_binding_handler_ != nullptr && !bindings.buffers.empty()) {
      buffer_binding_handler_(bindings.buffers);
    }
  }

 private:
  void Activate() const {
    CGCHECK(glIsProgram_(id_)) << "glIsProgram failed, glCreateProgram? not glDeleteProgram? id ~ " << id_;
    glUseProgram_(id_);
    texture_2_unit_.clear();
  }

  void SetBool(const std::string& location_name, bool value) const {
    glUniform1i_(GetUniformLocation(location_name), static_cast<int>(value));
  }

  void SetFloat(const std::string& location_name, float value) const {
    glUniform1f_(GetUniformLocation(location_name), value);
  }

  void SetInt(const std::string& location_name, int value) const {
    glUniform1i_(GetUniformLocation(location_name), value);
  }

  int BindTexture(const std::string& location_name, const Texture& value) const {
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

  void SetMat4(const std::string& location_name, const glm::mat4& value) const {
    glUniformMatrix4fv_(GetUniformLocation(location_name), 1, GL_FALSE, glm::value_ptr(value));
  }

  void SetVec4(const std::string& location_name, const glm::vec4& value) const {
    glUniform4fv_(GetUniformLocation(location_name), 1, glm::value_ptr(value));
  }

  void SetVec3(const std::string& location_name, const glm::vec3& value) const {
    glUniform3fv_(GetUniformLocation(location_name), 1, glm::value_ptr(value));
  }

  void SetVec2(const std::string& location_name, const glm::vec2& value) const {
    glUniform2fv_(GetUniformLocation(location_name), 1, glm::value_ptr(value));
  }

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
  BufferBindingHandler buffer_binding_handler_;
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

  std::shared_ptr<Program> CreateProgram(const ProgramDesc& desc) override {
    switch (desc.kind) {
      case ProgramKind::kRender:
        return std::make_shared<OpenGLProgram>(
            desc.name,
            desc.vs,
            desc.fs,
            desc.gs,
            desc.ts,
            [this](const std::vector<BufferBindingDesc>& bindings) { ApplyBufferBindings(bindings); });
      case ProgramKind::kCompute:
        return std::make_shared<OpenGLProgram>(
            desc.name,
            desc.cs,
            [this](const std::vector<BufferBindingDesc>& bindings) { ApplyBufferBindings(bindings); });
    }
    CGKILL("Unsupported ProgramKind");
    return nullptr;
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

  void ReadTextureData(const Texture& texture, const TextureReadDesc& desc, void* data) override {
    const GLenum target = ToGLTextureTarget(texture.meta().type);
    glBindTexture_(target, texture.id());
    glGetTexImage_(target,
                   desc.level,
                   ToGLPixelFormat(texture.meta().pixel_format),
                   ToGLPixelType(texture.meta().pixel_type),
                   data);
    glBindTexture_(target, 0);
  }

  void DispatchCompute(const ComputeDispatchDesc& desc) override {
    glDispatchCompute_(desc.workgroup_count.x, desc.workgroup_count.y, desc.workgroup_count.z);
    glMemoryBarrier_(ToGLMemoryBarrier(desc.barrier));
  }

  uint32_t CreateFramebuffer() override {
    uint32_t framebuffer = 0;
    glGenFramebuffers_(1, &framebuffer);
    framebuffer_records_.emplace(framebuffer, FramebufferRecord{});
    return framebuffer;
  }

  void DeleteFramebuffer(uint32_t framebuffer) override {
    glDeleteFramebuffers_(1, &framebuffer);
    framebuffer_records_.erase(framebuffer);
  }

  void ApplyFramebufferBinding(FramebufferBindPoint bind_point, uint32_t framebuffer) {
    glBindFramebuffer_(ToGLFramebufferBindPoint(bind_point), framebuffer);
  }

  bool ConfigureFramebuffer(uint32_t framebuffer, const FramebufferDesc& desc) override {
    CGCHECK(framebuffer != 0) << "Default framebuffer cannot be configured via FramebufferDesc";

    GLint previous_read_framebuffer = 0;
    GLint previous_draw_framebuffer = 0;
    glGetIntegerv_(GL_READ_FRAMEBUFFER_BINDING, &previous_read_framebuffer);
    glGetIntegerv_(GL_DRAW_FRAMEBUFFER_BINDING, &previous_draw_framebuffer);
    glBindFramebuffer_(GL_FRAMEBUFFER, framebuffer);

    FramebufferRecord& framebuffer_record = framebuffer_records_[framebuffer];
    for (const auto& attachment : framebuffer_record.attachments) {
      const AttachmentType attachment_type = static_cast<AttachmentType>(attachment.first & 0xffffffffu);
      const uint32_t attachment_index = static_cast<uint32_t>(attachment.first >> 32);
      glFramebufferTexture2D_(GL_FRAMEBUFFER, ToGLAttachment(attachment_type, attachment_index), GL_TEXTURE_2D, 0, 0);
    }
    framebuffer_record.attachments.clear();

    std::vector<GLenum> draw_buffers;
    GLenum read_buffer = GL_NONE;
    bool has_color_attachment = false;
    for (const FramebufferAttachmentDesc& attachment : desc.attachments) {
      CGCHECK(attachment.texture != nullptr) << "Framebuffer attachment texture must not be null";
      CGCHECK(attachment.texture->meta().type == Texture::kTexture2D)
          << "FramebufferDesc currently only supports Texture2D attachments";

      const GLenum gl_attachment = ToGLAttachment(attachment.attachment_type, attachment.attachment_index);
      glFramebufferTexture2D_(GL_FRAMEBUFFER, gl_attachment, GL_TEXTURE_2D, attachment.texture->id(), 0);
      framebuffer_record.attachments[AttachmentKey(attachment.attachment_type, attachment.attachment_index)] =
          attachment.texture->id();

      if (attachment.attachment_type == AttachmentType::kColor) {
        if (draw_buffers.size() <= attachment.attachment_index) {
          draw_buffers.resize(attachment.attachment_index + 1, GL_NONE);
        }
        draw_buffers[attachment.attachment_index] = gl_attachment;
        if (!has_color_attachment) {
          read_buffer = gl_attachment;
          has_color_attachment = true;
        }
      }
    }

    if (has_color_attachment) {
      glDrawBuffers_(draw_buffers.size(), draw_buffers.data());
      glReadBuffer_(read_buffer);
    } else {
      glDrawBuffer_(GL_NONE);
      glReadBuffer_(GL_NONE);
    }

    const bool is_complete = glCheckFramebufferStatus_(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    glBindFramebuffer_(GL_READ_FRAMEBUFFER, previous_read_framebuffer);
    glBindFramebuffer_(GL_DRAW_FRAMEBUFFER, previous_draw_framebuffer);
    return is_complete;
  }

  FramebufferState CaptureFramebufferState() const override {
    FramebufferState state;
    GLint viewport[4] = {};
    glGetIntegerv_(GL_VIEWPORT, viewport);
    glGetIntegerv_(GL_READ_FRAMEBUFFER_BINDING, &state.read_framebuffer);
    glGetIntegerv_(GL_DRAW_FRAMEBUFFER_BINDING, &state.draw_framebuffer);
    state.framebuffer = state.draw_framebuffer;
    state.viewport = glm::ivec4(viewport[0], viewport[1], viewport[2], viewport[3]);
    return state;
  }

  void RestoreFramebufferState(const FramebufferState& state) override {
    const bool legacy_state = state.read_framebuffer == 0 && state.draw_framebuffer == 0;
    const uint32_t read_framebuffer =
        static_cast<uint32_t>(std::max(legacy_state ? state.framebuffer : state.read_framebuffer, 0));
    const uint32_t draw_framebuffer =
        static_cast<uint32_t>(std::max(legacy_state ? state.framebuffer : state.draw_framebuffer, 0));
    if (read_framebuffer == draw_framebuffer) {
      ApplyFramebufferBinding(FramebufferBindPoint::kAll, draw_framebuffer);
    } else {
      ApplyFramebufferBinding(FramebufferBindPoint::kRead, read_framebuffer);
      ApplyFramebufferBinding(FramebufferBindPoint::kDraw, draw_framebuffer);
    }
    ApplyViewport({state.viewport.x, state.viewport.y}, {state.viewport.z, state.viewport.w});
  }

  void ApplyColorAttachmentClear(uint32_t attachment_index, const glm::vec4& color) {
    GLint draw_framebuffer = 0;
    glGetIntegerv_(GL_DRAW_FRAMEBUFFER_BINDING, &draw_framebuffer);
    if (draw_framebuffer == 0) {
      CGCHECK(attachment_index == 0)
          << "Default framebuffer only supports color attachment 0 for attachment-selective clear in OpenGL scene mode";
    } else {
      const auto framebuffer_it = framebuffer_records_.find(static_cast<uint32_t>(draw_framebuffer));
      CGCHECK(framebuffer_it != framebuffer_records_.end())
          << "Unknown framebuffer id " << draw_framebuffer << " in OpenGL scene mode";
      CGCHECK(framebuffer_it->second.attachments.contains(AttachmentKey(AttachmentType::kColor, attachment_index)))
          << "Attachment-selective clear requires color attachment "
          << attachment_index
          << " to exist on framebuffer "
          << draw_framebuffer
          << " in OpenGL scene mode";
    }
    glClearBufferfv_(GL_COLOR, attachment_index, glm::value_ptr(color));
  }

  void BeginRenderPass(const RenderPassDesc& desc) override {
    CGCHECK(desc.viewport_size.x > 0 && desc.viewport_size.y > 0)
        << "BeginRenderPass() requires a positive viewport size in OpenGL scene mode";
    ApplyFramebufferBinding(FramebufferBindPoint::kAll, desc.framebuffer);
    ApplyViewport(desc.viewport_origin, desc.viewport_size);

    const bool is_default_framebuffer = desc.framebuffer == 0;
    if (!is_default_framebuffer) {
      std::vector<GLenum> draw_buffers;
      GLenum read_buffer = GL_NONE;
      bool has_color_attachment = false;
      for (const RenderPassColorAttachmentDesc& color_attachment : desc.color_attachments) {
        if (draw_buffers.size() <= color_attachment.attachment_index) {
          draw_buffers.resize(color_attachment.attachment_index + 1, GL_NONE);
        }
        const GLenum gl_attachment = ToGLAttachment(AttachmentType::kColor, color_attachment.attachment_index);
        draw_buffers[color_attachment.attachment_index] = gl_attachment;
        if (!has_color_attachment) {
          read_buffer = gl_attachment;
          has_color_attachment = true;
        }
      }
      if (has_color_attachment) {
        glDrawBuffers_(draw_buffers.size(), draw_buffers.data());
        glReadBuffer_(read_buffer);
      } else {
        glDrawBuffer_(GL_NONE);
        glReadBuffer_(GL_NONE);
      }
    }

    ClearDesc clear_desc;
    if (desc.clear_depth) {
      clear_desc.mask = clear_desc.mask | ClearMask::kDepth;
      clear_desc.depth_clear_value = desc.depth_clear_value;
    }
    if (desc.clear_stencil) {
      clear_desc.mask = clear_desc.mask | ClearMask::kStencil;
      clear_desc.stencil_clear_value = desc.stencil_clear_value;
    }
    for (const RenderPassColorAttachmentDesc& color_attachment : desc.color_attachments) {
      if (color_attachment.clear) {
        if (is_default_framebuffer) {
          CGCHECK(color_attachment.attachment_index == 0)
              << "Default framebuffer only supports color attachment 0 in render pass abstraction";
          clear_desc.mask = clear_desc.mask | ClearMask::kColor;
          clear_desc.clear_color = color_attachment.clear_color;
        } else {
          ApplyColorAttachmentClear(color_attachment.attachment_index, color_attachment.clear_color);
        }
      }
    }

    if (clear_desc.mask != ClearMask::kNone) {
      Clear(clear_desc);
    }
  }

  void EndRenderPass() override {}

  void BlitFramebuffer(const BlitFramebufferDesc& desc) override {
    ValidateBlitFramebufferDesc(desc);
    GLint previous_read_framebuffer = 0;
    GLint previous_draw_framebuffer = 0;
    glGetIntegerv_(GL_READ_FRAMEBUFFER_BINDING, &previous_read_framebuffer);
    glGetIntegerv_(GL_DRAW_FRAMEBUFFER_BINDING, &previous_draw_framebuffer);
    const glm::ivec2 destination_size =
        (desc.draw_size.x > 0 || desc.draw_size.y > 0) ? desc.draw_size : desc.size;

    GLint source_read_buffer = 0;
    ApplyFramebufferBinding(FramebufferBindPoint::kRead, desc.read_framebuffer);
    glGetIntegerv_(GL_READ_BUFFER, &source_read_buffer);
    if (HasAnyFlag(desc.mask, ClearMask::kColor)) {
      if (desc.read_framebuffer == 0) {
        glReadBuffer_(ToGLReadBuffer(desc.read_buffer));
      } else {
        glReadBuffer_(ToGLAttachment(AttachmentType::kColor, desc.read_color_attachment_index));
      }
    }

    GLint destination_draw_buffer = 0;
    ApplyFramebufferBinding(FramebufferBindPoint::kDraw, desc.draw_framebuffer);
    glGetIntegerv_(GL_DRAW_BUFFER, &destination_draw_buffer);
    if (HasAnyFlag(desc.mask, ClearMask::kColor)) {
      if (desc.draw_framebuffer == 0) {
        glDrawBuffer_(ToGLReadBuffer(desc.draw_buffer));
      } else {
        glDrawBuffer_(ToGLAttachment(AttachmentType::kColor, desc.draw_color_attachment_index));
      }
    }

    glBlitFramebuffer_(desc.read_origin.x,
                       desc.read_origin.y,
                       desc.read_origin.x + desc.size.x,
                       desc.read_origin.y + desc.size.y,
                       desc.draw_origin.x,
                       desc.draw_origin.y,
                       desc.draw_origin.x + destination_size.x,
                       desc.draw_origin.y + destination_size.y,
                       ToGLClearMask(desc.mask),
                       ToGLBlitFilter(desc.filter));

    if (HasAnyFlag(desc.mask, ClearMask::kColor)) {
      glReadBuffer_(static_cast<GLenum>(source_read_buffer));
    }
    if (HasAnyFlag(desc.mask, ClearMask::kColor)) {
      glDrawBuffer_(static_cast<GLenum>(destination_draw_buffer));
    }
    ApplyFramebufferBinding(FramebufferBindPoint::kRead, previous_read_framebuffer);
    ApplyFramebufferBinding(FramebufferBindPoint::kDraw, previous_draw_framebuffer);
  }

  void Draw(const DrawDesc& desc) override {
    CGCHECK(desc.vertex_array != nullptr) << "Draw submission requires DrawDesc.vertex_array.";
    auto* gl_vertex_array = dynamic_cast<OpenGLVertexArray*>(desc.vertex_array);
    CGCHECK(gl_vertex_array != nullptr) << "OpenGLDevice expected OpenGLVertexArray for draw submission.";
    glBindVertexArray_(gl_vertex_array->id());
    if (desc.kind == DrawKind::kElements) {
      CGCHECK(desc.index_buffer != nullptr) << "Indexed draw submission requires DrawDesc.index_buffer.";
      auto* gl_index_buffer = dynamic_cast<OpenGLBuffer*>(desc.index_buffer);
      CGCHECK(gl_index_buffer != nullptr) << "OpenGLDevice expected OpenGLBuffer for indexed draw submission.";
      glBindBuffer_(gl_index_buffer->target(), gl_index_buffer->id());
    }
    const GLenum mode = ToGLPrimitiveTopology(desc.topology);
    if (desc.kind == DrawKind::kElements) {
      if (desc.instance_count > 1) {
        glDrawElementsInstanced_(mode, desc.count, GL_UNSIGNED_INT, nullptr, desc.instance_count);
      } else {
        glDrawElements_(mode, desc.count, GL_UNSIGNED_INT, nullptr);
      }
      return;
    }
    if (desc.instance_count > 1) {
      glDrawArraysInstanced_(mode, desc.first, desc.count, desc.instance_count);
    } else {
      glDrawArrays_(mode, desc.first, desc.count);
    }
  }

  void ReadPixels(const ReadPixelsDesc& desc, void* data) override {
    ValidateReadPixelsDesc(desc);
    GLint previous_read_framebuffer = 0;
    GLint previous_read_buffer = 0;
    GLint previous_pack_alignment = 0;
    glGetIntegerv_(GL_READ_FRAMEBUFFER_BINDING, &previous_read_framebuffer);
    glGetIntegerv_(GL_READ_BUFFER, &previous_read_buffer);
    glGetIntegerv_(GL_PACK_ALIGNMENT, &previous_pack_alignment);

    glBindFramebuffer_(GL_READ_FRAMEBUFFER, desc.framebuffer);
    if (desc.attachment_type == AttachmentType::kColor) {
      if (desc.framebuffer == 0) {
        glReadBuffer_(ToGLReadBuffer(desc.read_buffer));
      } else {
        glReadBuffer_(ToGLAttachment(desc.attachment_type, desc.attachment_index));
      }
    }
    glPixelStorei_(GL_PACK_ALIGNMENT, 1);
    glReadPixels_(desc.origin.x,
                  desc.origin.y,
                  desc.size.x,
                  desc.size.y,
                  ToGLPixelFormat(desc.format),
                  ToGLPixelType(desc.type),
                  data);

    glBindFramebuffer_(GL_READ_FRAMEBUFFER, previous_read_framebuffer);
    glReadBuffer_(static_cast<GLenum>(previous_read_buffer));
    glPixelStorei_(GL_PACK_ALIGNMENT, previous_pack_alignment);
  }

  void ApplyViewport(const glm::ivec2& origin, const glm::ivec2& size) {
    glViewport_(origin.x, origin.y, size.x, size.y);
  }

  void Clear(const ClearDesc& desc) override {
    if (HasAnyFlag(desc.mask, ClearMask::kColor)) {
      glClearColor_(desc.clear_color.r, desc.clear_color.g, desc.clear_color.b, desc.clear_color.a);
    }
    if (HasAnyFlag(desc.mask, ClearMask::kDepth)) {
      glClearDepth_(desc.depth_clear_value);
    }
    if (HasAnyFlag(desc.mask, ClearMask::kStencil)) {
      glClearStencil_(desc.stencil_clear_value);
    }
    if (desc.mask != ClearMask::kNone) {
      glClear_(ToGLClearMask(desc.mask));
    }
  }

  void ApplyRenderState(const RenderStateDesc& state) override {
    if (state.depth_test_enabled.has_value()) {
      ApplyDepthTestEnabled(*state.depth_test_enabled);
    }
    if (state.cull_enabled.has_value()) {
      ApplyCullEnabled(*state.cull_enabled);
    }
    if (state.cull_mode.has_value()) {
      ApplyCullMode(*state.cull_mode);
    }
    if (state.front_face.has_value()) {
      ApplyFrontFace(*state.front_face);
    }
  }

  RenderState CaptureRenderState() const override {
    return {
        .depth_test_enabled = depth_test_enabled_,
        .cull_enabled = cull_enabled_,
        .cull_mode = cull_mode_,
        .front_face = front_face_,
    };
  }

  void RestoreRenderState(const RenderState& state) override {
    ApplyDepthTestEnabled(state.depth_test_enabled);
    ApplyCullEnabled(state.cull_enabled);
    ApplyCullMode(state.cull_mode);
    ApplyFrontFace(state.front_face);
  }

  void ApplyDepthTestEnabled(bool enabled) {
    depth_test_enabled_ = enabled;
    if (enabled) {
      glEnable_(GL_DEPTH_TEST);
    } else {
      glDisable_(GL_DEPTH_TEST);
    }
  }

  void ApplyCullEnabled(bool enabled) {
    cull_enabled_ = enabled;
    if (enabled) {
      glEnable_(GL_CULL_FACE);
    } else {
      glDisable_(GL_CULL_FACE);
    }
  }

  void ApplyCullMode(CullMode mode) {
    cull_mode_ = mode;
    glCullFace_(ToGLCullMode(mode));
  }

  void ApplyFrontFace(FrontFace winding) {
    front_face_ = winding;
    glFrontFace_(ToGLFrontFace(winding));
  }

 private:
  struct FramebufferRecord {
    std::unordered_map<uint64_t, uint32_t> attachments;
  };

  void ValidateReadPixelsDesc(const ReadPixelsDesc& desc) const {
    ValidateFramebufferAttachmentAccess(
        desc.framebuffer, desc.attachment_type, desc.attachment_index, "ReadPixels()", "read");
  }

  void ValidateBlitFramebufferDesc(const BlitFramebufferDesc& desc) const {
    if (HasAnyFlag(desc.mask, ClearMask::kColor)) {
      ValidateFramebufferAttachmentAccess(desc.read_framebuffer,
                                          AttachmentType::kColor,
                                          desc.read_color_attachment_index,
                                          "BlitFramebuffer()",
                                          "source");
      ValidateFramebufferAttachmentAccess(desc.draw_framebuffer,
                                          AttachmentType::kColor,
                                          desc.draw_color_attachment_index,
                                          "BlitFramebuffer()",
                                          "destination");
    }
    if (HasAnyFlag(desc.mask, ClearMask::kDepth)) {
      ValidateFramebufferAttachmentAccess(
          desc.read_framebuffer, AttachmentType::kDepth, 0, "BlitFramebuffer()", "source");
      ValidateFramebufferAttachmentAccess(
          desc.draw_framebuffer, AttachmentType::kDepth, 0, "BlitFramebuffer()", "destination");
    }
    if (HasAnyFlag(desc.mask, ClearMask::kStencil)) {
      ValidateFramebufferAttachmentAccess(
          desc.read_framebuffer, AttachmentType::kStencil, 0, "BlitFramebuffer()", "source");
      ValidateFramebufferAttachmentAccess(
          desc.draw_framebuffer, AttachmentType::kStencil, 0, "BlitFramebuffer()", "destination");
    }
  }

  void ValidateFramebufferAttachmentAccess(uint32_t framebuffer,
                                           AttachmentType attachment_type,
                                           uint32_t attachment_index,
                                           const char* operation_name,
                                           const char* attachment_role) const {
    if (framebuffer == 0) {
      CGCHECK(attachment_index == 0)
          << "Default framebuffer only supports " << AttachmentTypeName(attachment_type)
          << " attachment 0 for " << operation_name << " in OpenGL scene mode";
      return;
    }
    const auto framebuffer_it = framebuffer_records_.find(framebuffer);
    CGCHECK(framebuffer_it != framebuffer_records_.end()) << "Unknown framebuffer id " << framebuffer
                                                          << " in OpenGL scene mode";
    CGCHECK(framebuffer_it->second.attachments.contains(AttachmentKey(attachment_type, attachment_index)))
        << operation_name << " requires " << attachment_role << " " << AttachmentTypeName(attachment_type)
        << " attachment " << attachment_index << " to exist on framebuffer " << framebuffer
        << " in OpenGL scene mode";
  }

  static const char* AttachmentTypeName(AttachmentType attachment_type) {
    switch (attachment_type) {
      case AttachmentType::kColor:
        return "color";
      case AttachmentType::kDepth:
        return "depth";
      case AttachmentType::kStencil:
        return "stencil";
    }
    return "unknown";
  }

  Capabilities capabilities_;
  std::unordered_map<uint32_t, FramebufferRecord> framebuffer_records_;
  bool depth_test_enabled_ = false;
  bool cull_enabled_ = false;
  CullMode cull_mode_ = CullMode::kBack;
  FrontFace front_face_ = FrontFace::kCounterClockwise;
};

}  // namespace

std::unique_ptr<Device> CreateOpenGLDevice() {
  return std::make_unique<OpenGLDevice>();
}

void InitializeOpenGLDevice() {
  SetDevice(CreateOpenGLDevice());
}

}  // namespace cg::rhi
