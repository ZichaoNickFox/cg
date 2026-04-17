#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "rhi/types.h"

namespace cg {
class Texture;
}

namespace cg::rhi {

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

enum class MapAccess {
  kReadOnly = 0,
};

enum class TextureAccess {
  kReadOnly = 0,
  kWriteOnly = 1,
  kReadWrite = 2,
};

enum class MemoryBarrier {
  kAll = 0,
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

struct FramebufferState {
  int32_t framebuffer = 0;
  glm::ivec4 viewport = glm::ivec4(0, 0, 0, 0);
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

class Buffer {
 public:
  virtual ~Buffer() = default;

  virtual BufferType type() const = 0;
  virtual void SetData(size_t size_in_bytes, const void* data, BufferUsage usage) = 0;
  virtual void UpdateData(size_t offset_in_bytes, size_t size_in_bytes, const void* data) = 0;
  virtual void Bind() = 0;
  virtual void Unbind() = 0;
  virtual void BindBase(uint32_t binding_point) = 0;
  virtual void* Map(MapAccess access) = 0;
  virtual void Unmap() = 0;
};

class VertexArray {
 public:
  virtual ~VertexArray() = default;

  virtual void Bind() = 0;
  virtual void Unbind() = 0;
  virtual void EnableAttribute(uint32_t index) = 0;
  virtual void SetFloatAttribute(uint32_t index,
                                 int component_count,
                                 int stride_in_bytes,
                                 size_t offset_in_bytes) = 0;
  virtual void SetAttributeDivisor(uint32_t index, uint32_t divisor) = 0;
};

class Program {
 public:
  virtual ~Program() = default;

  virtual uint32_t id() const = 0;
  virtual void Use() const = 0;
  virtual void SetBool(const std::string& location_name, bool value) const = 0;
  virtual void SetFloat(const std::string& location_name, float value) const = 0;
  virtual void SetInt(const std::string& location_name, int value) const = 0;
  virtual int BindTexture(const std::string& location_name, const Texture& value) const = 0;
  virtual void SetMat4(const std::string& location_name, const glm::mat4& value) const = 0;
  virtual void SetVec4(const std::string& location_name, const glm::vec4& value) const = 0;
  virtual void SetVec3(const std::string& location_name, const glm::vec3& value) const = 0;
  virtual void SetVec2(const std::string& location_name, const glm::vec2& value) const = 0;
};

class Device {
 public:
  virtual ~Device() = default;

  virtual const Capabilities& capabilities() const = 0;
  virtual std::unique_ptr<Buffer> CreateBuffer(BufferType type) = 0;
  virtual std::unique_ptr<VertexArray> CreateVertexArray() = 0;
  virtual std::shared_ptr<Program> CreateRenderProgram(const std::string& name,
                                                       const std::vector<ShaderCodePart>& vs,
                                                       const std::vector<ShaderCodePart>& fs,
                                                       const std::vector<ShaderCodePart>& gs,
                                                       const std::vector<ShaderCodePart>& ts) = 0;
  virtual std::shared_ptr<Program> CreateComputeProgram(const std::string& name,
                                                        const std::vector<ShaderCodePart>& cs) = 0;
  virtual void EnsureTextureUploaded(Texture* texture) = 0;
  virtual void ReleaseTexture(Texture* texture) = 0;
  virtual void ReadTextureData(const Texture& texture, int level, void* data, size_t size_in_bytes) = 0;
  virtual void BindStorageTexture(uint32_t texture_unit, const Texture& texture, TextureAccess access) = 0;
  virtual void DispatchCompute(const glm::uvec3& workgroup_count) = 0;
  virtual void MemoryBarrier(MemoryBarrier barrier) = 0;
  virtual uint32_t CreateFramebuffer() = 0;
  virtual void DeleteFramebuffer(uint32_t framebuffer) = 0;
  virtual void BindFramebuffer(FramebufferBindPoint bind_point, uint32_t framebuffer) = 0;
  virtual void AttachFramebufferTexture2D(AttachmentType attachment_type,
                                          uint32_t attachment_index,
                                          const Texture& texture) = 0;
  virtual bool CheckFramebufferComplete() = 0;
  virtual FramebufferState CaptureFramebufferState() const = 0;
  virtual void RestoreFramebufferState(const FramebufferState& state) = 0;
  virtual void ClearColorAttachment(uint32_t attachment_index, const glm::vec4& color) = 0;
  virtual void SetDrawBuffers(const std::vector<uint32_t>& color_attachment_indices) = 0;
  virtual void BlitFramebuffer(uint32_t read_framebuffer,
                               uint32_t draw_framebuffer,
                               const glm::ivec2& size,
                               ClearMask mask,
                               FilterMode filter) = 0;
  virtual void DrawArrays(PrimitiveTopology topology,
                          uint32_t first,
                          uint32_t count,
                          uint32_t instance_count = 1) = 0;
  virtual void DrawElements(PrimitiveTopology topology,
                            uint32_t count,
                            uint32_t instance_count = 1) = 0;
  virtual void SetReadBuffer(ReadBuffer buffer) = 0;
  virtual void ReadPixels(const glm::ivec2& origin,
                          const glm::ivec2& size,
                          PixelFormat format,
                          PixelType type,
                          void* data) = 0;
  virtual void SetViewport(const glm::ivec2& origin, const glm::ivec2& size) = 0;
  virtual void SetClearColor(const glm::vec4& color) = 0;
  virtual void Clear(ClearMask mask) = 0;
  virtual void SetDepthTestEnabled(bool enabled) = 0;
  virtual void SetCullEnabled(bool enabled) = 0;
  virtual void SetCullMode(CullMode mode) = 0;
  virtual void SetFrontFace(FrontFace winding) = 0;
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
