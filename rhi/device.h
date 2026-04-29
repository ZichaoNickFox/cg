#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <glm/glm.hpp>

#include "rhi/types.h"

namespace cg {
class Texture;
}

namespace cg::rhi {

class Buffer;
class VertexArray;

enum class BackendType {
  kUnknown = 0,
  kOpenGL = 1,
  kVulkan = 2,
  kMetal = 3,
};

enum class BufferType {
  kVertex = 0,
  kIndex = 1,
  kStorage = 2,
  kAtomicCounter = 3,
  kTexture = 4,
};

enum class BufferUsage {
  kStatic = 0,
  kDynamic = 1,
  kStreamCopy = 2,
};

enum class TextureAccess {
  kReadOnly = 0,
  kWriteOnly = 1,
  kReadWrite = 2,
};

enum class MemoryBarrier {
  kAll = 0,
};

struct ComputeDispatchDesc {
  glm::uvec3 workgroup_count = glm::uvec3(0);
  MemoryBarrier barrier = MemoryBarrier::kAll;
};

struct Capabilities {
  BackendType backend = BackendType::kUnknown;
  BackendType scene_backend = BackendType::kUnknown;
  std::string graphics_api_name;
  std::string scene_api_name;
  std::string shader_language_name;
  bool supports_glsl_450 = false;
  bool supports_storage_buffers = false;
  bool supports_storage_images = false;
  bool supports_compute = false;
};

struct ShaderCodePart {
  std::string glsl_path;
  std::string code;
};

enum class ProgramKind {
  kRender = 0,
  kCompute = 1,
};

struct ProgramDesc {
  std::string name;
  ProgramKind kind = ProgramKind::kRender;
  std::vector<ShaderCodePart> vs;
  std::vector<ShaderCodePart> fs;
  std::vector<ShaderCodePart> gs;
  std::vector<ShaderCodePart> ts;
  std::vector<ShaderCodePart> cs;
};

struct FramebufferState {
  int32_t read_framebuffer = 0;
  int32_t draw_framebuffer = 0;
  int32_t framebuffer = 0;
  glm::ivec4 viewport = glm::ivec4(0, 0, 0, 0);
};

struct RenderPassColorAttachmentDesc {
  uint32_t attachment_index = 0;
  bool clear = false;
  glm::vec4 clear_color = glm::vec4(0.0f);
};

struct RenderPassDesc {
  uint32_t framebuffer = 0;
  glm::ivec2 viewport_origin = glm::ivec2(0, 0);
  glm::ivec2 viewport_size = glm::ivec2(0, 0);
  bool clear_depth = false;
  bool clear_stencil = false;
  float depth_clear_value = 1.0f;
  int stencil_clear_value = 0;
  std::vector<RenderPassColorAttachmentDesc> color_attachments;
};

struct ClearDesc {
  ClearMask mask = ClearMask::kNone;
  glm::vec4 clear_color = glm::vec4(0.0f);
  float depth_clear_value = 1.0f;
  int stencil_clear_value = 0;
};

enum class DrawKind {
  kArrays = 0,
  kElements = 1,
};

struct DrawDesc {
  DrawKind kind = DrawKind::kArrays;
  PrimitiveTopology topology = PrimitiveTopology::kTriangles;
  VertexArray* vertex_array = nullptr;
  Buffer* index_buffer = nullptr;
  uint32_t first = 0;
  uint32_t count = 0;
  uint32_t instance_count = 1;
};

struct VertexAttributeDesc {
  uint32_t index = 0;
  int component_count = 0;
  int stride_in_bytes = 0;
  size_t offset_in_bytes = 0;
  uint32_t divisor = 0;
};

struct VertexArrayBindingDesc {
  Buffer* buffer = nullptr;
  std::vector<VertexAttributeDesc> attributes;
};

struct BufferReadDesc {
  size_t offset_in_bytes = 0;
  size_t size_in_bytes = 0;
};

struct TextureReadDesc {
  int level = 0;
  size_t size_in_bytes = 0;
};

using ProgramUniformValue = std::variant<bool, float, int, glm::mat4, glm::vec4, glm::vec3, glm::vec2>;

struct ProgramUniformBindingDesc {
  std::string name;
  ProgramUniformValue value;
};

struct ProgramTextureBindingDesc {
  std::string name;
  const Texture* texture = nullptr;
};

struct ProgramStorageTextureBindingDesc {
  std::string name;
  const Texture* texture = nullptr;
  TextureAccess access = TextureAccess::kReadOnly;
};

struct BufferBindingDesc {
  Buffer* buffer = nullptr;
  uint32_t binding_point = 0;
};

struct ProgramBindings {
  std::vector<ProgramUniformBindingDesc> uniforms;
  std::vector<ProgramTextureBindingDesc> textures;
  std::vector<ProgramStorageTextureBindingDesc> storage_textures;
  std::vector<BufferBindingDesc> buffers;
};

struct RenderStateDesc {
  std::optional<bool> depth_test_enabled;
  std::optional<bool> cull_enabled;
  std::optional<CullMode> cull_mode;
  std::optional<FrontFace> front_face;
};

struct RenderState {
  bool depth_test_enabled = false;
  bool cull_enabled = false;
  CullMode cull_mode = CullMode::kBack;
  FrontFace front_face = FrontFace::kCounterClockwise;
};

enum class FramebufferBindPoint {
  kAll = 0,
  kRead = 1,
  kDraw = 2,
};

enum class AttachmentType {
  kColor = 0,
  kDepth = 1,
  kStencil = 2,
};

enum class ReadBuffer {
  kFront = 0,
  kBack = 1,
};

struct FramebufferAttachmentDesc {
  AttachmentType attachment_type = AttachmentType::kColor;
  uint32_t attachment_index = 0;
  const Texture* texture = nullptr;
};

struct FramebufferDesc {
  std::vector<FramebufferAttachmentDesc> attachments;
};

struct ReadPixelsDesc {
  uint32_t framebuffer = 0;
  AttachmentType attachment_type = AttachmentType::kColor;
  uint32_t attachment_index = 0;
  ReadBuffer read_buffer = ReadBuffer::kBack;
  glm::ivec2 origin = glm::ivec2(0, 0);
  glm::ivec2 size = glm::ivec2(0, 0);
  PixelFormat format = PixelFormat::kRGBA;
  PixelType type = PixelType::kUInt8;
};

struct BlitFramebufferDesc {
  uint32_t read_framebuffer = 0;
  uint32_t draw_framebuffer = 0;
  uint32_t read_color_attachment_index = 0;
  uint32_t draw_color_attachment_index = 0;
  ReadBuffer read_buffer = ReadBuffer::kBack;
  ReadBuffer draw_buffer = ReadBuffer::kBack;
  glm::ivec2 read_origin = glm::ivec2(0, 0);
  glm::ivec2 draw_origin = glm::ivec2(0, 0);
  glm::ivec2 size = glm::ivec2(0, 0);
  glm::ivec2 draw_size = glm::ivec2(0, 0);
  ClearMask mask = ClearMask::kColor;
  FilterMode filter = FilterMode::kNearest;
};

class Buffer {
 public:
  virtual ~Buffer() = default;

  virtual BufferType type() const = 0;
  virtual void SetData(size_t size_in_bytes, const void* data, BufferUsage usage) = 0;
  virtual void UpdateData(size_t offset_in_bytes, size_t size_in_bytes, const void* data) = 0;
  virtual void BindBase(uint32_t binding_point) = 0;
  virtual void ReadData(const BufferReadDesc& desc, void* data) = 0;
};

class VertexArray {
 public:
  virtual ~VertexArray() = default;

  virtual void ApplyBinding(const VertexArrayBindingDesc& desc) = 0;
};

class Program {
 public:
  virtual ~Program() = default;

  virtual void ApplyBindings(const ProgramBindings& bindings) const = 0;
};

class Device {
 public:
  virtual ~Device() = default;

  virtual const Capabilities& capabilities() const = 0;
  virtual std::unique_ptr<Buffer> CreateBuffer(BufferType type) = 0;
  virtual std::unique_ptr<VertexArray> CreateVertexArray() = 0;
  virtual std::shared_ptr<Program> CreateProgram(const ProgramDesc& desc) = 0;
  virtual void EnsureTextureUploaded(Texture* texture) = 0;
  virtual void ReleaseTexture(Texture* texture) = 0;
  virtual void ReadTextureData(const Texture& texture, const TextureReadDesc& desc, void* data) = 0;
  virtual void DispatchCompute(const ComputeDispatchDesc& desc) = 0;
  virtual uint32_t CreateFramebuffer() = 0;
  virtual void DeleteFramebuffer(uint32_t framebuffer) = 0;
  virtual bool ConfigureFramebuffer(uint32_t framebuffer, const FramebufferDesc& desc) = 0;
  virtual FramebufferState CaptureFramebufferState() const = 0;
  virtual void RestoreFramebufferState(const FramebufferState& state) = 0;
  virtual void BeginRenderPass(const RenderPassDesc& desc) = 0;
  virtual void EndRenderPass() = 0;
  virtual void BlitFramebuffer(const BlitFramebufferDesc& desc) = 0;
  virtual void Draw(const DrawDesc& desc) = 0;
  virtual void ReadPixels(const ReadPixelsDesc& desc, void* data) = 0;
  virtual void Clear(const ClearDesc& desc) = 0;
  virtual void ApplyRenderState(const RenderStateDesc& state) = 0;
  virtual RenderState CaptureRenderState() const = 0;
  virtual void RestoreRenderState(const RenderState& state) = 0;

  void ApplyBufferBindings(const std::vector<BufferBindingDesc>& bindings) {
    DidApplyBufferBindings(bindings);
    for (const BufferBindingDesc& binding : bindings) {
      if (binding.buffer == nullptr) {
        continue;
      }
      binding.buffer->BindBase(binding.binding_point);
    }
  }

  void DrawBindings(const Program& program, const ProgramBindings& bindings, const DrawDesc& desc) {
    program.ApplyBindings(bindings);
    Draw(desc);
  }

  void DispatchComputeBindings(const Program& program,
                               const ProgramBindings& bindings,
                               const ComputeDispatchDesc& desc) {
    program.ApplyBindings(bindings);
    DispatchCompute(desc);
  }

 protected:
  virtual void DidApplyBufferBindings(const std::vector<BufferBindingDesc>& bindings) {
    (void)bindings;
  }
};

class ScopedRenderPass {
 public:
  ScopedRenderPass() = default;

  ScopedRenderPass(Device& device, const RenderPassDesc& desc) : device_(&device), active_(true) {
    device_->BeginRenderPass(desc);
  }

  ~ScopedRenderPass() {
    End();
  }

  ScopedRenderPass(const ScopedRenderPass&) = delete;
  ScopedRenderPass& operator=(const ScopedRenderPass&) = delete;

  ScopedRenderPass(ScopedRenderPass&& other) noexcept : device_(other.device_), active_(other.active_) {
    other.device_ = nullptr;
    other.active_ = false;
  }

  ScopedRenderPass& operator=(ScopedRenderPass&& other) noexcept {
    if (this != &other) {
      End();
      device_ = other.device_;
      active_ = other.active_;
      other.device_ = nullptr;
      other.active_ = false;
    }
    return *this;
  }

  void End() {
    if (active_ && device_ != nullptr) {
      device_->EndRenderPass();
      active_ = false;
    }
  }

  bool active() const { return active_; }

 private:
  Device* device_ = nullptr;
  bool active_ = false;
};

class ScopedFramebufferState {
 public:
  ScopedFramebufferState() = default;

  explicit ScopedFramebufferState(Device& device)
      : device_(&device), active_(true), state_(device.CaptureFramebufferState()) {}

  ~ScopedFramebufferState() {
    Restore();
  }

  ScopedFramebufferState(const ScopedFramebufferState&) = delete;
  ScopedFramebufferState& operator=(const ScopedFramebufferState&) = delete;

  ScopedFramebufferState(ScopedFramebufferState&& other) noexcept
      : device_(other.device_), active_(other.active_), state_(other.state_) {
    other.device_ = nullptr;
    other.active_ = false;
  }

  ScopedFramebufferState& operator=(ScopedFramebufferState&& other) noexcept {
    if (this != &other) {
      Restore();
      device_ = other.device_;
      active_ = other.active_;
      state_ = other.state_;
      other.device_ = nullptr;
      other.active_ = false;
    }
    return *this;
  }

  void Restore() {
    if (active_ && device_ != nullptr) {
      device_->RestoreFramebufferState(state_);
      active_ = false;
    }
  }

  bool active() const { return active_; }
  const FramebufferState& captured_state() const { return state_; }

 private:
  Device* device_ = nullptr;
  bool active_ = false;
  FramebufferState state_;
};

class ScopedRenderState {
 public:
  ScopedRenderState() = default;

  explicit ScopedRenderState(Device& device)
      : device_(&device), active_(true), state_(device.CaptureRenderState()) {}

  ~ScopedRenderState() {
    Restore();
  }

  ScopedRenderState(const ScopedRenderState&) = delete;
  ScopedRenderState& operator=(const ScopedRenderState&) = delete;

  ScopedRenderState(ScopedRenderState&& other) noexcept
      : device_(other.device_), active_(other.active_), state_(other.state_) {
    other.device_ = nullptr;
    other.active_ = false;
  }

  ScopedRenderState& operator=(ScopedRenderState&& other) noexcept {
    if (this != &other) {
      Restore();
      device_ = other.device_;
      active_ = other.active_;
      state_ = other.state_;
      other.device_ = nullptr;
      other.active_ = false;
    }
    return *this;
  }

  void Restore() {
    if (active_ && device_ != nullptr) {
      device_->RestoreRenderState(state_);
      active_ = false;
    }
  }

  bool active() const { return active_; }
  const RenderState& captured_state() const { return state_; }

 private:
  Device* device_ = nullptr;
  bool active_ = false;
  RenderState state_;
};

bool HasDevice();
void SetDevice(std::unique_ptr<Device> device);
Device& GetDevice();
const Capabilities& GetCapabilities();
BackendType GetSceneBackendType();
std::string GetSceneApiName();

std::unique_ptr<Device> CreateOpenGLDevice();
void InitializeOpenGLDevice();
std::unique_ptr<Device> CreateVulkanDevice(bool enable_opengl_scene_compatibility = false);
void InitializeVulkanDevice(bool enable_opengl_scene_compatibility = false);

}  // namespace cg::rhi
