#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "rhi/device.h"

namespace cg::test {

struct FakeBufferState {
  rhi::BufferType type = rhi::BufferType::kVertex;
  rhi::BufferUsage last_usage = rhi::BufferUsage::kStatic;
  std::vector<uint8_t> data;
  std::vector<uint32_t> bound_base_points;
  int set_data_call_count = 0;
  int update_data_call_count = 0;
  int bind_call_count = 0;
  int unbind_call_count = 0;
  int bind_base_call_count = 0;
  int map_call_count = 0;
  int unmap_call_count = 0;
};

struct FakeVertexAttributeState {
  int component_count = 0;
  int stride_in_bytes = 0;
  size_t offset_in_bytes = 0;
  uint32_t divisor = 0;
  int enable_call_count = 0;
};

struct FakeVertexArrayState {
  std::unordered_map<uint32_t, FakeVertexAttributeState> attributes;
  int bind_call_count = 0;
  int unbind_call_count = 0;
};

struct FakeDrawCall {
  enum class Kind {
    kArrays = 0,
    kElements = 1,
  };

  Kind kind = Kind::kArrays;
  rhi::PrimitiveTopology topology = rhi::PrimitiveTopology::kTriangles;
  uint32_t first = 0;
  uint32_t count = 0;
  uint32_t instance_count = 1;
};

class FakeBuffer final : public rhi::Buffer {
 public:
  explicit FakeBuffer(std::shared_ptr<FakeBufferState> state) : state_(std::move(state)) {}

  rhi::BufferType type() const override { return state_->type; }

  void SetData(size_t size_in_bytes, const void* data, rhi::BufferUsage usage) override {
    state_->last_usage = usage;
    state_->data.assign(size_in_bytes, 0);
    if (data != nullptr && size_in_bytes > 0) {
      std::memcpy(state_->data.data(), data, size_in_bytes);
    }
    ++state_->set_data_call_count;
  }

  void UpdateData(size_t offset_in_bytes, size_t size_in_bytes, const void* data) override {
    if (state_->data.size() < offset_in_bytes + size_in_bytes) {
      state_->data.resize(offset_in_bytes + size_in_bytes, 0);
    }
    if (data != nullptr && size_in_bytes > 0) {
      std::memcpy(state_->data.data() + offset_in_bytes, data, size_in_bytes);
    }
    ++state_->update_data_call_count;
  }

  void BindBase(uint32_t binding_point) override {
    state_->bound_base_points.push_back(binding_point);
    ++state_->bind_base_call_count;
  }

  void Bind() override { ++state_->bind_call_count; }

  void Unbind() override { ++state_->unbind_call_count; }

  void* Map(rhi::MapAccess access) override {
    last_map_access_ = access;
    ++state_->map_call_count;
    return state_->data.empty() ? nullptr : state_->data.data();
  }

  void Unmap() override { ++state_->unmap_call_count; }

 private:
  std::shared_ptr<FakeBufferState> state_;
  std::optional<rhi::MapAccess> last_map_access_;
};

class FakeVertexArray final : public rhi::VertexArray {
 public:
  explicit FakeVertexArray(std::shared_ptr<FakeVertexArrayState> state) : state_(std::move(state)) {}

  void Bind() override { ++state_->bind_call_count; }
  void Unbind() override { ++state_->unbind_call_count; }

  void EnableAttribute(uint32_t index) override {
    ++state_->attributes[index].enable_call_count;
  }

  void SetFloatAttribute(uint32_t index,
                         int component_count,
                         int stride_in_bytes,
                         size_t offset_in_bytes) override {
    auto& attribute = state_->attributes[index];
    attribute.component_count = component_count;
    attribute.stride_in_bytes = stride_in_bytes;
    attribute.offset_in_bytes = offset_in_bytes;
  }

  void SetAttributeDivisor(uint32_t index, uint32_t divisor) override {
    state_->attributes[index].divisor = divisor;
  }

 private:
  std::shared_ptr<FakeVertexArrayState> state_;
};

class FakeProgram final : public rhi::Program {
 public:
  explicit FakeProgram(uint32_t id) : id_(id) {}

  uint32_t id() const override { return id_; }
  void Use() const override {}
  void SetBool(const std::string& location_name, bool value) const override {}
  void SetFloat(const std::string& location_name, float value) const override {}
  void SetInt(const std::string& location_name, int value) const override {}
  int BindTexture(const std::string& location_name, const Texture& value) const override { return 0; }
  void SetMat4(const std::string& location_name, const glm::mat4& value) const override {}
  void SetVec4(const std::string& location_name, const glm::vec4& value) const override {}
  void SetVec3(const std::string& location_name, const glm::vec3& value) const override {}
  void SetVec2(const std::string& location_name, const glm::vec2& value) const override {}

 private:
  uint32_t id_ = 0;
};

class FakeDevice : public rhi::Device {
 public:
  explicit FakeDevice(rhi::Capabilities capabilities = DefaultCapabilities())
      : capabilities_(std::move(capabilities)) {}

  static rhi::Capabilities DefaultCapabilities() {
    rhi::Capabilities capabilities;
    capabilities.backend = rhi::BackendType::kOpenGL;
    capabilities.graphics_api_name = "Fake OpenGL";
    capabilities.shader_language_name = "Fake GLSL";
    capabilities.supports_glsl_450 = true;
    capabilities.supports_storage_buffers = true;
    capabilities.supports_storage_images = true;
    capabilities.supports_compute = true;
    return capabilities;
  }

  const rhi::Capabilities& capabilities() const override { return capabilities_; }

  std::unique_ptr<rhi::Buffer> CreateBuffer(rhi::BufferType type) override {
    auto state = std::make_shared<FakeBufferState>();
    state->type = type;
    created_buffers_.push_back(state);
    return std::make_unique<FakeBuffer>(state);
  }

  std::unique_ptr<rhi::VertexArray> CreateVertexArray() override {
    auto state = std::make_shared<FakeVertexArrayState>();
    created_vertex_arrays_.push_back(state);
    return std::make_unique<FakeVertexArray>(state);
  }

  std::shared_ptr<rhi::Program> CreateRenderProgram(const std::string& name,
                                                    const std::vector<rhi::ShaderCodePart>& vs,
                                                    const std::vector<rhi::ShaderCodePart>& fs,
                                                    const std::vector<rhi::ShaderCodePart>& gs,
                                                    const std::vector<rhi::ShaderCodePart>& ts) override {
    return std::make_shared<FakeProgram>(++next_program_id_);
  }

  std::shared_ptr<rhi::Program> CreateComputeProgram(const std::string& name,
                                                     const std::vector<rhi::ShaderCodePart>& cs) override {
    return std::make_shared<FakeProgram>(++next_program_id_);
  }

  void EnsureTextureUploaded(Texture* texture) override {}
  void ReleaseTexture(Texture* texture) override {}
  void ReadTextureData(const Texture& texture, int level, void* data, size_t size_in_bytes) override {}

  void BindStorageTexture(uint32_t texture_unit, const Texture& texture, rhi::TextureAccess access) override {}
  void DispatchCompute(const glm::uvec3& workgroup_count) override {}
  void MemoryBarrier(rhi::MemoryBarrier barrier) override {}

  uint32_t CreateFramebuffer() override { return ++next_framebuffer_id_; }
  void DeleteFramebuffer(uint32_t framebuffer) override {}

  void BindFramebuffer(rhi::FramebufferBindPoint bind_point, uint32_t framebuffer) override {
    framebuffer_state_.framebuffer = static_cast<int32_t>(framebuffer);
  }

  void AttachFramebufferTexture2D(rhi::AttachmentType attachment_type,
                                  uint32_t attachment_index,
                                  const Texture& texture) override {}

  bool CheckFramebufferComplete() override { return true; }

  rhi::FramebufferState CaptureFramebufferState() const override { return framebuffer_state_; }

  void RestoreFramebufferState(const rhi::FramebufferState& state) override { framebuffer_state_ = state; }

  void ClearColorAttachment(uint32_t attachment_index, const glm::vec4& color) override {}
  void SetDrawBuffers(const std::vector<uint32_t>& color_attachment_indices) override {}

  void BlitFramebuffer(uint32_t read_framebuffer,
                       uint32_t draw_framebuffer,
                       const glm::ivec2& size,
                       rhi::ClearMask mask,
                       rhi::FilterMode filter) override {}

  void DrawArrays(rhi::PrimitiveTopology topology,
                  uint32_t first,
                  uint32_t count,
                  uint32_t instance_count) override {
    draw_calls_.push_back({
        .kind = FakeDrawCall::Kind::kArrays,
        .topology = topology,
        .first = first,
        .count = count,
        .instance_count = instance_count,
    });
  }

  void DrawElements(rhi::PrimitiveTopology topology,
                    uint32_t count,
                    uint32_t instance_count) override {
    draw_calls_.push_back({
        .kind = FakeDrawCall::Kind::kElements,
        .topology = topology,
        .first = 0,
        .count = count,
        .instance_count = instance_count,
    });
  }

  void SetReadBuffer(rhi::ReadBuffer buffer) override {}

  void ReadPixels(const glm::ivec2& origin,
                  const glm::ivec2& size,
                  rhi::PixelFormat format,
                  rhi::PixelType type,
                  void* data) override {}

  void SetViewport(const glm::ivec2& origin, const glm::ivec2& size) override {
    framebuffer_state_.viewport = glm::ivec4(origin, size);
  }

  void SetClearColor(const glm::vec4& color) override {}
  void Clear(rhi::ClearMask mask) override {}
  void SetDepthTestEnabled(bool enabled) override {}
  void SetCullEnabled(bool enabled) override {}
  void SetCullMode(rhi::CullMode mode) override {}
  void SetFrontFace(rhi::FrontFace winding) override {}

  const std::vector<std::shared_ptr<FakeBufferState>>& created_buffers() const { return created_buffers_; }
  const std::vector<std::shared_ptr<FakeVertexArrayState>>& created_vertex_arrays() const {
    return created_vertex_arrays_;
  }
  const std::vector<FakeDrawCall>& draw_calls() const { return draw_calls_; }

 private:
  rhi::Capabilities capabilities_;
  rhi::FramebufferState framebuffer_state_;
  std::vector<std::shared_ptr<FakeBufferState>> created_buffers_;
  std::vector<std::shared_ptr<FakeVertexArrayState>> created_vertex_arrays_;
  std::vector<FakeDrawCall> draw_calls_;
  uint32_t next_program_id_ = 0;
  uint32_t next_framebuffer_id_ = 0;
};

class ScopedFakeDevice {
 public:
  explicit ScopedFakeDevice(rhi::Capabilities capabilities = FakeDevice::DefaultCapabilities()) {
    auto device = std::make_unique<FakeDevice>(std::move(capabilities));
    device_ = device.get();
    rhi::SetDevice(std::move(device));
  }

  ~ScopedFakeDevice() { rhi::SetDevice(std::unique_ptr<rhi::Device>()); }

  FakeDevice& device() { return *device_; }
  const FakeDevice& device() const { return *device_; }

 private:
  FakeDevice* device_ = nullptr;
};

template <typename ValueType>
ValueType ReadScalar(const std::vector<uint8_t>& data, size_t offset = 0) {
  ValueType value{};
  std::memcpy(&value, data.data() + offset, sizeof(ValueType));
  return value;
}

}  // namespace cg::test
