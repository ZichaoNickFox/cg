#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "rhi/device.h"

namespace cg::test {

inline uint32_t TextureToken(const Texture& texture) {
  return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&texture));
}

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
  int read_data_call_count = 0;
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
  std::vector<rhi::VertexArrayBindingDesc> binding_descs;
  int apply_binding_call_count = 0;
  int bind_call_count = 0;
  int unbind_call_count = 0;
};

struct FakeDrawCall {
  rhi::DrawKind kind = rhi::DrawKind::kArrays;
  rhi::PrimitiveTopology topology = rhi::PrimitiveTopology::kTriangles;
  bool has_vertex_array = false;
  bool has_index_buffer = false;
  uint32_t first = 0;
  uint32_t count = 0;
  uint32_t instance_count = 1;
};

struct FakeRenderPassState {
  rhi::RenderPassDesc last_desc;
  int begin_call_count = 0;
  int end_call_count = 0;
};

struct FakeFramebufferConfigState {
  uint32_t framebuffer = 0;
  rhi::FramebufferDesc last_desc;
  int configure_call_count = 0;
};

struct FakeReadPixelsState {
  rhi::ReadPixelsDesc last_desc;
  int call_count = 0;
};

struct FakeTextureReadState {
  rhi::TextureReadDesc last_desc;
  int call_count = 0;
};

struct FakeBlitFramebufferState {
  rhi::BlitFramebufferDesc last_desc;
  int call_count = 0;
};

struct FakeClearState {
  rhi::ClearDesc last_desc;
  int call_count = 0;
};

struct FakeDispatchComputeState {
  rhi::ComputeDispatchDesc last_desc;
  int call_count = 0;
};

struct FakeProgramState {
  uint32_t id = 0;
  rhi::ProgramDesc create_desc;
  std::unordered_map<std::string, bool> bool_uniforms;
  std::unordered_map<std::string, float> float_uniforms;
  std::unordered_map<std::string, int> int_uniforms;
  std::unordered_map<std::string, glm::mat4> mat4_uniforms;
  std::unordered_map<std::string, glm::vec4> vec4_uniforms;
  std::unordered_map<std::string, glm::vec3> vec3_uniforms;
  std::unordered_map<std::string, glm::vec2> vec2_uniforms;
  std::unordered_map<std::string, uint32_t> texture_ids;
  int activation_call_count = 0;
  int apply_bindings_call_count = 0;
};

struct FakeStorageTextureBindingCall {
  uint32_t texture_unit = 0;
  uint32_t texture_id = 0;
  rhi::TextureAccess access = rhi::TextureAccess::kReadOnly;
};

struct FakeBufferBindingCall {
  rhi::BufferType buffer_type = rhi::BufferType::kVertex;
  uint32_t binding_point = 0;
};

struct FakeBufferBindingState {
  std::vector<FakeBufferBindingCall> calls;
  int apply_call_count = 0;
};

struct FakeRenderState {
  bool depth_test_enabled = false;
  bool cull_enabled = false;
  rhi::CullMode cull_mode = rhi::CullMode::kBack;
  rhi::FrontFace front_face = rhi::FrontFace::kCounterClockwise;
  int apply_call_count = 0;
};

class FakeBuffer final : public rhi::Buffer {
 public:
  using BindBaseHandler = std::function<void(rhi::BufferType type, uint32_t binding_point)>;

  explicit FakeBuffer(std::shared_ptr<FakeBufferState> state, BindBaseHandler bind_base_handler = {})
      : state_(std::move(state)), bind_base_handler_(std::move(bind_base_handler)) {}

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
    if (bind_base_handler_ != nullptr) {
      bind_base_handler_(state_->type, binding_point);
    }
  }

  void ReadData(const rhi::BufferReadDesc& desc, void* data) override {
    CGCHECK(data != nullptr);
    CGCHECK(desc.offset_in_bytes + desc.size_in_bytes <= state_->data.size());
    std::memcpy(data, state_->data.data() + desc.offset_in_bytes, desc.size_in_bytes);
    ++state_->read_data_call_count;
  }

  void MarkDrawBound() {
    ++state_->bind_call_count;
  }

 private:
  std::shared_ptr<FakeBufferState> state_;
  BindBaseHandler bind_base_handler_;
};

class FakeVertexArray final : public rhi::VertexArray {
 public:
  explicit FakeVertexArray(std::shared_ptr<FakeVertexArrayState> state) : state_(std::move(state)) {}

  void ApplyBinding(const rhi::VertexArrayBindingDesc& desc) override {
    state_->binding_descs.push_back(desc);
    ++state_->apply_binding_call_count;
    for (const rhi::VertexAttributeDesc& attribute_desc : desc.attributes) {
      auto& attribute = state_->attributes[attribute_desc.index];
      attribute.component_count = attribute_desc.component_count;
      attribute.stride_in_bytes = attribute_desc.stride_in_bytes;
      attribute.offset_in_bytes = attribute_desc.offset_in_bytes;
      attribute.divisor = attribute_desc.divisor;
      ++attribute.enable_call_count;
    }
  }

  void MarkDrawBound() {
    ++state_->bind_call_count;
  }

 private:
  void EnableAttribute(uint32_t index) {
    ++state_->attributes[index].enable_call_count;
  }

  void SetFloatAttribute(uint32_t index,
                         int component_count,
                         int stride_in_bytes,
                         size_t offset_in_bytes) {
    auto& attribute = state_->attributes[index];
    attribute.component_count = component_count;
    attribute.stride_in_bytes = stride_in_bytes;
    attribute.offset_in_bytes = offset_in_bytes;
  }

  void SetAttributeDivisor(uint32_t index, uint32_t divisor) {
    state_->attributes[index].divisor = divisor;
  }
  std::shared_ptr<FakeVertexArrayState> state_;
};

class FakeProgram final : public rhi::Program {
 public:
  using StorageTextureBindingHandler =
      std::function<void(uint32_t texture_unit, const Texture& texture, rhi::TextureAccess access)>;
  using BufferBindingHandler = std::function<void(const std::vector<rhi::BufferBindingDesc>& bindings)>;

  explicit FakeProgram(std::shared_ptr<FakeProgramState> state,
                       StorageTextureBindingHandler storage_texture_binding_handler = {},
                       BufferBindingHandler buffer_binding_handler = {})
      : state_(std::move(state)),
        storage_texture_binding_handler_(std::move(storage_texture_binding_handler)),
        buffer_binding_handler_(std::move(buffer_binding_handler)) {}

  void ApplyBindings(const rhi::ProgramBindings& bindings) const override {
    Activate();
    ++state_->apply_bindings_call_count;
    for (const rhi::ProgramUniformBindingDesc& uniform : bindings.uniforms) {
      std::visit(
          [&](const auto& value) {
            using ValueType = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<ValueType, bool>) {
              SetBool(uniform.name, value);
            } else if constexpr (std::is_same_v<ValueType, float>) {
              SetFloat(uniform.name, value);
            } else if constexpr (std::is_same_v<ValueType, int>) {
              SetInt(uniform.name, value);
            } else if constexpr (std::is_same_v<ValueType, glm::mat4>) {
              SetMat4(uniform.name, value);
            } else if constexpr (std::is_same_v<ValueType, glm::vec4>) {
              SetVec4(uniform.name, value);
            } else if constexpr (std::is_same_v<ValueType, glm::vec3>) {
              SetVec3(uniform.name, value);
            } else if constexpr (std::is_same_v<ValueType, glm::vec2>) {
              SetVec2(uniform.name, value);
            }
          },
          uniform.value);
    }
    for (const rhi::ProgramTextureBindingDesc& texture_binding : bindings.textures) {
      if (texture_binding.texture != nullptr) {
        BindTexture(texture_binding.name, *texture_binding.texture);
      }
    }
    for (const rhi::ProgramStorageTextureBindingDesc& storage_texture_binding : bindings.storage_textures) {
      if (storage_texture_binding.texture == nullptr) {
        continue;
      }
      const int texture_unit = BindTexture(storage_texture_binding.name, *storage_texture_binding.texture);
      if (storage_texture_binding_handler_ != nullptr) {
        storage_texture_binding_handler_(texture_unit,
                                         *storage_texture_binding.texture,
                                         storage_texture_binding.access);
      }
    }
    if (buffer_binding_handler_ != nullptr && !bindings.buffers.empty()) {
      buffer_binding_handler_(bindings.buffers);
    }
  }

 private:
  void Activate() const {
    texture_2_unit_.clear();
    ++state_->activation_call_count;
  }

  void SetBool(const std::string& location_name, bool value) const {
    state_->bool_uniforms[location_name] = value;
  }

  void SetFloat(const std::string& location_name, float value) const {
    state_->float_uniforms[location_name] = value;
  }

  void SetInt(const std::string& location_name, int value) const {
    state_->int_uniforms[location_name] = value;
  }

  int BindTexture(const std::string& location_name, const Texture& value) const {
    int unit = 0;
    const uint32_t texture_token = TextureToken(value);
    auto iter = texture_2_unit_.find(texture_token);
    if (iter != texture_2_unit_.end()) {
      unit = iter->second;
    } else {
      unit = static_cast<int>(texture_2_unit_.size());
      texture_2_unit_[texture_token] = unit;
    }
    state_->texture_ids[location_name] = texture_token;
    return unit;
  }

  void SetMat4(const std::string& location_name, const glm::mat4& value) const {
    state_->mat4_uniforms[location_name] = value;
  }

  void SetVec4(const std::string& location_name, const glm::vec4& value) const {
    state_->vec4_uniforms[location_name] = value;
  }

  void SetVec3(const std::string& location_name, const glm::vec3& value) const {
    state_->vec3_uniforms[location_name] = value;
  }

  void SetVec2(const std::string& location_name, const glm::vec2& value) const {
    state_->vec2_uniforms[location_name] = value;
  }

  std::shared_ptr<FakeProgramState> state_;
  StorageTextureBindingHandler storage_texture_binding_handler_;
  BufferBindingHandler buffer_binding_handler_;
  mutable std::unordered_map<uint32_t, int> texture_2_unit_;
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

  std::shared_ptr<rhi::Program> CreateProgram(const rhi::ProgramDesc& desc) override {
    auto state = std::make_shared<FakeProgramState>();
    state->id = ++next_program_id_;
    state->create_desc = desc;
    created_programs_.push_back(state);
    return std::make_shared<FakeProgram>(
        state,
        [this](uint32_t texture_unit, const Texture& texture, rhi::TextureAccess access) {
          RecordStorageTextureBinding(texture_unit, texture, access);
        },
        [this](const std::vector<rhi::BufferBindingDesc>& bindings) {
          ApplyBufferBindings(bindings);
        });
  }

  void EnsureTextureUploaded(Texture* texture) override {}
  void ReleaseTexture(Texture* texture) override {}
  void ReadTextureData(const Texture& texture, const rhi::TextureReadDesc& desc, void* data) override {
    RecordTextureRead(desc);
  }

  void RecordStorageTextureBinding(uint32_t texture_unit, const Texture& texture, rhi::TextureAccess access) {
    storage_texture_binding_calls_.push_back({
        .texture_unit = texture_unit,
        .texture_id = TextureToken(texture),
        .access = access,
    });
  }
  void RecordBufferBindings(const std::vector<rhi::BufferBindingDesc>& bindings) {
    ++buffer_binding_state_.apply_call_count;
    for (const rhi::BufferBindingDesc& binding : bindings) {
      if (binding.buffer == nullptr) {
        continue;
      }
      buffer_binding_state_.calls.push_back({
          .buffer_type = binding.buffer->type(),
          .binding_point = binding.binding_point,
      });
    }
  }
  void DispatchCompute(const rhi::ComputeDispatchDesc& desc) override {
    dispatch_compute_state_.last_desc = desc;
    ++dispatch_compute_state_.call_count;
  }

  uint32_t CreateFramebuffer() override { return ++next_framebuffer_id_; }
  void DeleteFramebuffer(uint32_t framebuffer) override {}

  bool ConfigureFramebuffer(uint32_t framebuffer, const rhi::FramebufferDesc& desc) override {
    framebuffer_config_state_.framebuffer = framebuffer;
    framebuffer_config_state_.last_desc = desc;
    ++framebuffer_config_state_.configure_call_count;
    return !desc.attachments.empty();
  }

  rhi::FramebufferState CaptureFramebufferState() const override { return framebuffer_state_; }

  void RestoreFramebufferState(const rhi::FramebufferState& state) override { framebuffer_state_ = state; }

  void BeginRenderPass(const rhi::RenderPassDesc& desc) override {
    render_pass_state_.last_desc = desc;
    framebuffer_state_.read_framebuffer = static_cast<int32_t>(desc.framebuffer);
    framebuffer_state_.draw_framebuffer = static_cast<int32_t>(desc.framebuffer);
    framebuffer_state_.framebuffer = static_cast<int32_t>(desc.framebuffer);
    framebuffer_state_.viewport =
        glm::ivec4(desc.viewport_origin.x, desc.viewport_origin.y, desc.viewport_size.x, desc.viewport_size.y);
    ++render_pass_state_.begin_call_count;
  }

  void EndRenderPass() override {
    ++render_pass_state_.end_call_count;
  }

  void BlitFramebuffer(const rhi::BlitFramebufferDesc& desc) override {
    blit_framebuffer_state_.last_desc = desc;
    ++blit_framebuffer_state_.call_count;
  }

  void Draw(const rhi::DrawDesc& desc) override {
    CGCHECK(desc.vertex_array != nullptr) << "Draw submission requires DrawDesc.vertex_array.";
    auto* vertex_array = dynamic_cast<FakeVertexArray*>(desc.vertex_array);
    CGCHECK(vertex_array != nullptr) << "FakeDevice expected FakeVertexArray for draw submission.";
    vertex_array->MarkDrawBound();
    if (desc.kind == rhi::DrawKind::kElements) {
      CGCHECK(desc.index_buffer != nullptr) << "Indexed draw submission requires DrawDesc.index_buffer.";
      auto* index_buffer = dynamic_cast<FakeBuffer*>(desc.index_buffer);
      CGCHECK(index_buffer != nullptr) << "FakeDevice expected FakeBuffer for indexed draw submission.";
      index_buffer->MarkDrawBound();
    }
    draw_calls_.push_back({
        .kind = desc.kind,
        .topology = desc.topology,
        .has_vertex_array = desc.vertex_array != nullptr,
        .has_index_buffer = desc.index_buffer != nullptr,
        .first = desc.first,
        .count = desc.count,
        .instance_count = desc.instance_count,
    });
  }

  void ReadPixels(const rhi::ReadPixelsDesc& desc, void* data) override {
    (void)data;
    read_pixels_state_.last_desc = desc;
    ++read_pixels_state_.call_count;
  }

  void Clear(const rhi::ClearDesc& desc) override {
    clear_state_.last_desc = desc;
    ++clear_state_.call_count;
  }
  void ApplyRenderState(const rhi::RenderStateDesc& state) override {
    if (state.depth_test_enabled.has_value()) {
      render_state_.depth_test_enabled = *state.depth_test_enabled;
    }
    if (state.cull_enabled.has_value()) {
      render_state_.cull_enabled = *state.cull_enabled;
    }
    if (state.cull_mode.has_value()) {
      render_state_.cull_mode = *state.cull_mode;
    }
    if (state.front_face.has_value()) {
      render_state_.front_face = *state.front_face;
    }
    ++render_state_.apply_call_count;
  }
  rhi::RenderState CaptureRenderState() const override {
    return {
        .depth_test_enabled = render_state_.depth_test_enabled,
        .cull_enabled = render_state_.cull_enabled,
        .cull_mode = render_state_.cull_mode,
        .front_face = render_state_.front_face,
    };
  }
  void RestoreRenderState(const rhi::RenderState& state) override {
    render_state_.depth_test_enabled = state.depth_test_enabled;
    render_state_.cull_enabled = state.cull_enabled;
    render_state_.cull_mode = state.cull_mode;
    render_state_.front_face = state.front_face;
  }

  const std::vector<std::shared_ptr<FakeBufferState>>& created_buffers() const { return created_buffers_; }
  const std::vector<std::shared_ptr<FakeVertexArrayState>>& created_vertex_arrays() const {
    return created_vertex_arrays_;
  }
  const std::vector<std::shared_ptr<FakeProgramState>>& created_programs() const { return created_programs_; }
  const FakeProgramState& last_program_state() const { return *created_programs_.back(); }
  const std::vector<FakeStorageTextureBindingCall>& storage_texture_binding_calls() const {
    return storage_texture_binding_calls_;
  }
  const FakeBufferBindingState& buffer_binding_state() const { return buffer_binding_state_; }
  const FakeRenderState& render_state() const { return render_state_; }
  const std::vector<FakeDrawCall>& draw_calls() const { return draw_calls_; }
  const FakeFramebufferConfigState& framebuffer_config_state() const { return framebuffer_config_state_; }
  const FakeBlitFramebufferState& blit_framebuffer_state() const { return blit_framebuffer_state_; }
  const FakeReadPixelsState& read_pixels_state() const { return read_pixels_state_; }
  const FakeTextureReadState& texture_read_state() const { return texture_read_state_; }
  const FakeClearState& clear_state() const { return clear_state_; }
  const FakeRenderPassState& render_pass_state() const { return render_pass_state_; }
  const FakeDispatchComputeState& dispatch_compute_state() const { return dispatch_compute_state_; }

 protected:
  void RecordTextureRead(const rhi::TextureReadDesc& desc) {
    texture_read_state_.last_desc = desc;
    ++texture_read_state_.call_count;
  }

 private:
  void DidApplyBufferBindings(const std::vector<rhi::BufferBindingDesc>& bindings) override {
    RecordBufferBindings(bindings);
  }

  rhi::Capabilities capabilities_;
  rhi::FramebufferState framebuffer_state_;
  std::vector<std::shared_ptr<FakeBufferState>> created_buffers_;
  std::vector<std::shared_ptr<FakeVertexArrayState>> created_vertex_arrays_;
  std::vector<std::shared_ptr<FakeProgramState>> created_programs_;
  std::vector<FakeStorageTextureBindingCall> storage_texture_binding_calls_;
  FakeBufferBindingState buffer_binding_state_;
  FakeRenderState render_state_;
  std::vector<FakeDrawCall> draw_calls_;
  FakeFramebufferConfigState framebuffer_config_state_;
  FakeBlitFramebufferState blit_framebuffer_state_;
  FakeReadPixelsState read_pixels_state_;
  FakeTextureReadState texture_read_state_;
  FakeClearState clear_state_;
  FakeRenderPassState render_pass_state_;
  FakeDispatchComputeState dispatch_compute_state_;
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
