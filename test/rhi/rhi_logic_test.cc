#include "gtest/gtest.h"

#include <array>
#include <cstdint>
#include <memory>

#include "rhi/device.h"
#include "rhi/gl/types.h"
#include "rhi/types.h"
#include "test/test_support/fake_device.h"

namespace {

using cg::rhi::BackendType;
using cg::rhi::ClearMask;

TEST(rhi_logic_test, ClearMaskBitOperatorsComposeAndFilterFlags) {
  const ClearMask mask = ClearMask::kColor | ClearMask::kDepth;

  EXPECT_TRUE(cg::rhi::HasAnyFlag(mask, ClearMask::kColor));
  EXPECT_TRUE(cg::rhi::HasAnyFlag(mask, ClearMask::kDepth));
  EXPECT_FALSE(cg::rhi::HasAnyFlag(mask, ClearMask::kStencil));
  EXPECT_EQ(static_cast<uint32_t>(mask & ClearMask::kDepth), static_cast<uint32_t>(ClearMask::kDepth));
  EXPECT_EQ(static_cast<uint32_t>(mask & ClearMask::kStencil), 0u);
}

TEST(rhi_logic_test, ClearDescFlowsThroughActiveDeviceAsExplicitContract) {
  cg::test::ScopedFakeDevice scoped_device;

  cg::rhi::ClearDesc clear_desc;
  clear_desc.mask = ClearMask::kColor | ClearMask::kStencil;
  clear_desc.clear_color = {0.25f, 0.5f, 0.75f, 1.0f};
  clear_desc.depth_clear_value = 0.125f;
  clear_desc.stencil_clear_value = 7;

  cg::rhi::GetDevice().Clear(clear_desc);

  const cg::test::FakeClearState& clear_state = scoped_device.device().clear_state();
  EXPECT_EQ(clear_state.call_count, 1);
  EXPECT_EQ(clear_state.last_desc.mask, clear_desc.mask);
  EXPECT_FLOAT_EQ(clear_state.last_desc.clear_color.r, clear_desc.clear_color.r);
  EXPECT_FLOAT_EQ(clear_state.last_desc.clear_color.g, clear_desc.clear_color.g);
  EXPECT_FLOAT_EQ(clear_state.last_desc.clear_color.b, clear_desc.clear_color.b);
  EXPECT_FLOAT_EQ(clear_state.last_desc.clear_color.a, clear_desc.clear_color.a);
  EXPECT_FLOAT_EQ(clear_state.last_desc.depth_clear_value, clear_desc.depth_clear_value);
  EXPECT_EQ(clear_state.last_desc.stencil_clear_value, clear_desc.stencil_clear_value);
}

TEST(rhi_logic_test, ComputeDispatchDescFlowsThroughActiveDeviceAsExplicitContract) {
  cg::test::ScopedFakeDevice scoped_device;

  cg::rhi::ComputeDispatchDesc dispatch_desc;
  dispatch_desc.workgroup_count = glm::uvec3(2, 3, 4);
  dispatch_desc.barrier = cg::rhi::MemoryBarrier::kAll;

  cg::rhi::GetDevice().DispatchCompute(dispatch_desc);

  const cg::test::FakeDispatchComputeState& dispatch_state = scoped_device.device().dispatch_compute_state();
  EXPECT_EQ(dispatch_state.call_count, 1);
  EXPECT_EQ(dispatch_state.last_desc.workgroup_count, dispatch_desc.workgroup_count);
  EXPECT_EQ(dispatch_state.last_desc.barrier, dispatch_desc.barrier);
}

TEST(rhi_logic_test, DeviceDrawBindingsSubmitsProgramBindingsAndDrawTogether) {
  cg::test::ScopedFakeDevice scoped_device;

  std::shared_ptr<cg::rhi::Program> program = cg::rhi::GetDevice().CreateProgram({
      .name = "fake_draw",
      .kind = cg::rhi::ProgramKind::kRender,
  });
  ASSERT_TRUE(program != nullptr);

  std::unique_ptr<cg::rhi::VertexArray> vertex_array = cg::rhi::GetDevice().CreateVertexArray();
  ASSERT_TRUE(vertex_array != nullptr);

  cg::rhi::ProgramBindings bindings;
  bindings.uniforms.push_back({"material_index", 9});

  cg::rhi::GetDevice().DrawBindings(*program,
                                    bindings,
                                    {
                                        .kind = cg::rhi::DrawKind::kArrays,
                                        .topology = cg::rhi::PrimitiveTopology::kTriangles,
                                        .vertex_array = vertex_array.get(),
                                        .first = 1,
                                        .count = 6,
                                        .instance_count = 2,
                                    });

  const cg::test::FakeProgramState& program_state = scoped_device.device().last_program_state();
  EXPECT_EQ(program_state.activation_call_count, 1);
  EXPECT_EQ(program_state.apply_bindings_call_count, 1);
  EXPECT_EQ(program_state.int_uniforms.at("material_index"), 9);

  const auto& draw_calls = scoped_device.device().draw_calls();
  ASSERT_EQ(draw_calls.size(), 1u);
  EXPECT_EQ(draw_calls.front().kind, cg::rhi::DrawKind::kArrays);
  EXPECT_EQ(draw_calls.front().topology, cg::rhi::PrimitiveTopology::kTriangles);
  EXPECT_TRUE(draw_calls.front().has_vertex_array);
  EXPECT_FALSE(draw_calls.front().has_index_buffer);
  EXPECT_EQ(draw_calls.front().first, 1u);
  EXPECT_EQ(draw_calls.front().count, 6u);
  EXPECT_EQ(draw_calls.front().instance_count, 2u);
}

TEST(rhi_logic_test, DeviceDispatchComputeBindingsSubmitsProgramBindingsAndDispatchTogether) {
  cg::test::ScopedFakeDevice scoped_device;

  std::shared_ptr<cg::rhi::Program> program = cg::rhi::GetDevice().CreateProgram({
      .name = "fake_compute",
      .kind = cg::rhi::ProgramKind::kCompute,
  });
  ASSERT_TRUE(program != nullptr);

  cg::rhi::ProgramBindings bindings;
  bindings.uniforms.push_back({"frame_num", 12});

  cg::rhi::ComputeDispatchDesc dispatch_desc;
  dispatch_desc.workgroup_count = glm::uvec3(4, 3, 2);
  dispatch_desc.barrier = cg::rhi::MemoryBarrier::kAll;
  cg::rhi::GetDevice().DispatchComputeBindings(*program, bindings, dispatch_desc);

  const cg::test::FakeProgramState& program_state = scoped_device.device().last_program_state();
  EXPECT_EQ(program_state.activation_call_count, 1);
  EXPECT_EQ(program_state.apply_bindings_call_count, 1);
  EXPECT_EQ(program_state.int_uniforms.at("frame_num"), 12);

  const cg::test::FakeDispatchComputeState& dispatch_state = scoped_device.device().dispatch_compute_state();
  EXPECT_EQ(dispatch_state.call_count, 1);
  EXPECT_EQ(dispatch_state.last_desc.workgroup_count, dispatch_desc.workgroup_count);
  EXPECT_EQ(dispatch_state.last_desc.barrier, dispatch_desc.barrier);
}

TEST(rhi_logic_test, DrawDescFlowsThroughActiveDeviceAsExplicitContract) {
  cg::test::ScopedFakeDevice scoped_device;

  std::unique_ptr<cg::rhi::VertexArray> vertex_array = cg::rhi::GetDevice().CreateVertexArray();
  ASSERT_TRUE(vertex_array != nullptr);
  std::unique_ptr<cg::rhi::Buffer> index_buffer = cg::rhi::GetDevice().CreateBuffer(cg::rhi::BufferType::kIndex);
  ASSERT_TRUE(index_buffer != nullptr);

  cg::rhi::DrawDesc draw_desc;
  draw_desc.kind = cg::rhi::DrawKind::kElements;
  draw_desc.topology = cg::rhi::PrimitiveTopology::kTriangleStrip;
  draw_desc.vertex_array = vertex_array.get();
  draw_desc.index_buffer = index_buffer.get();
  draw_desc.first = 4;
  draw_desc.count = 12;
  draw_desc.instance_count = 3;

  cg::rhi::GetDevice().Draw(draw_desc);

  const auto& draw_calls = scoped_device.device().draw_calls();
  ASSERT_EQ(draw_calls.size(), 1u);
  EXPECT_EQ(draw_calls.front().kind, draw_desc.kind);
  EXPECT_EQ(draw_calls.front().topology, draw_desc.topology);
  EXPECT_TRUE(draw_calls.front().has_vertex_array);
  EXPECT_TRUE(draw_calls.front().has_index_buffer);
  EXPECT_EQ(draw_calls.front().first, draw_desc.first);
  EXPECT_EQ(draw_calls.front().count, draw_desc.count);
  EXPECT_EQ(draw_calls.front().instance_count, draw_desc.instance_count);

  const auto& created_vertex_arrays = scoped_device.device().created_vertex_arrays();
  ASSERT_EQ(created_vertex_arrays.size(), 1u);
  EXPECT_EQ(created_vertex_arrays.front()->bind_call_count, 1);

  const auto& created_buffers = scoped_device.device().created_buffers();
  ASSERT_EQ(created_buffers.size(), 1u);
  EXPECT_EQ(created_buffers.front()->bind_call_count, 1);
}

TEST(rhi_logic_test, DrawRejectsMissingVertexArrayInExplicitSubmissionContract) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  EXPECT_DEATH(
      {
        cg::test::ScopedFakeDevice scoped_device;
        cg::rhi::GetDevice().Draw({
            .kind = cg::rhi::DrawKind::kArrays,
            .topology = cg::rhi::PrimitiveTopology::kTriangles,
            .first = 0,
            .count = 3,
        });
      },
      ".*DrawDesc\\.vertex_array.*");
}

TEST(rhi_logic_test, DrawRejectsMissingIndexBufferForIndexedExplicitSubmissionContract) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  EXPECT_DEATH(
      {
        cg::test::ScopedFakeDevice scoped_device;
        std::unique_ptr<cg::rhi::VertexArray> vertex_array = cg::rhi::GetDevice().CreateVertexArray();
        cg::rhi::GetDevice().Draw({
            .kind = cg::rhi::DrawKind::kElements,
            .topology = cg::rhi::PrimitiveTopology::kTriangles,
            .vertex_array = vertex_array.get(),
            .count = 3,
        });
      },
      ".*DrawDesc\\.index_buffer.*");
}

TEST(rhi_logic_test, VertexArrayBindingDescFlowsThroughActiveDeviceAsExplicitContract) {
  cg::test::ScopedFakeDevice scoped_device;

  std::unique_ptr<cg::rhi::VertexArray> vertex_array = cg::rhi::GetDevice().CreateVertexArray();
  ASSERT_TRUE(vertex_array != nullptr);
  std::unique_ptr<cg::rhi::Buffer> vertex_buffer = cg::rhi::GetDevice().CreateBuffer(cg::rhi::BufferType::kVertex);
  ASSERT_TRUE(vertex_buffer != nullptr);

  cg::rhi::VertexArrayBindingDesc binding_desc;
  binding_desc.buffer = vertex_buffer.get();
  binding_desc.attributes = {{
      .index = 2,
      .component_count = 3,
      .stride_in_bytes = 24,
      .offset_in_bytes = 12,
      .divisor = 4,
  }};

  vertex_array->ApplyBinding(binding_desc);

  const auto& created_vertex_arrays = scoped_device.device().created_vertex_arrays();
  ASSERT_EQ(created_vertex_arrays.size(), 1u);
  EXPECT_EQ(created_vertex_arrays.front()->apply_binding_call_count, 1);
  ASSERT_EQ(created_vertex_arrays.front()->binding_descs.size(), 1u);
  EXPECT_EQ(created_vertex_arrays.front()->binding_descs.front().buffer, binding_desc.buffer);
  ASSERT_EQ(created_vertex_arrays.front()->binding_descs.front().attributes.size(), 1u);
  EXPECT_EQ(created_vertex_arrays.front()->binding_descs.front().attributes.front().index, 2u);
  ASSERT_TRUE(created_vertex_arrays.front()->attributes.contains(2u));
  EXPECT_EQ(created_vertex_arrays.front()->attributes.at(2u).component_count, 3);
  EXPECT_EQ(created_vertex_arrays.front()->attributes.at(2u).stride_in_bytes, 24);
  EXPECT_EQ(created_vertex_arrays.front()->attributes.at(2u).offset_in_bytes, 12u);
  EXPECT_EQ(created_vertex_arrays.front()->attributes.at(2u).divisor, 4u);
}

TEST(rhi_logic_test, BufferReadDescFlowsThroughActiveDeviceAsExplicitContract) {
  cg::test::ScopedFakeDevice scoped_device;

  std::unique_ptr<cg::rhi::Buffer> buffer = cg::rhi::GetDevice().CreateBuffer(cg::rhi::BufferType::kStorage);
  ASSERT_TRUE(buffer != nullptr);
  const std::array<uint32_t, 4> words = {7u, 8u, 9u, 10u};
  buffer->SetData(sizeof(words), words.data(), cg::rhi::BufferUsage::kStatic);

  std::array<uint32_t, 2> readback = {};
  const cg::rhi::BufferReadDesc read_desc{
      .offset_in_bytes = sizeof(uint32_t),
      .size_in_bytes = sizeof(readback),
  };
  buffer->ReadData(read_desc, readback.data());

  const auto& created_buffers = scoped_device.device().created_buffers();
  ASSERT_EQ(created_buffers.size(), 1u);
  EXPECT_EQ(created_buffers.front()->read_data_call_count, 1);
  EXPECT_EQ(readback[0], 8u);
  EXPECT_EQ(readback[1], 9u);
}

TEST(rhi_logic_test, BufferBindingDescFlowsThroughDeviceAsExplicitContract) {
  cg::test::ScopedFakeDevice scoped_device;

  std::unique_ptr<cg::rhi::Buffer> storage_buffer = cg::rhi::GetDevice().CreateBuffer(cg::rhi::BufferType::kStorage);
  ASSERT_TRUE(storage_buffer != nullptr);
  std::unique_ptr<cg::rhi::Buffer> counter_buffer =
      cg::rhi::GetDevice().CreateBuffer(cg::rhi::BufferType::kAtomicCounter);
  ASSERT_TRUE(counter_buffer != nullptr);

  cg::rhi::GetDevice().ApplyBufferBindings({
      {
          .buffer = storage_buffer.get(),
          .binding_point = 4,
      },
      {
          .buffer = counter_buffer.get(),
          .binding_point = 9,
      },
  });

  const cg::test::FakeBufferBindingState& buffer_binding_state = scoped_device.device().buffer_binding_state();
  EXPECT_EQ(buffer_binding_state.apply_call_count, 1);
  ASSERT_EQ(buffer_binding_state.calls.size(), 2u);
  EXPECT_EQ(buffer_binding_state.calls[0].buffer_type, cg::rhi::BufferType::kStorage);
  EXPECT_EQ(buffer_binding_state.calls[0].binding_point, 4u);
  EXPECT_EQ(buffer_binding_state.calls[1].buffer_type, cg::rhi::BufferType::kAtomicCounter);
  EXPECT_EQ(buffer_binding_state.calls[1].binding_point, 9u);

  const auto& created_buffers = scoped_device.device().created_buffers();
  ASSERT_EQ(created_buffers.size(), 2u);
  ASSERT_EQ(created_buffers[0]->bound_base_points.size(), 1u);
  EXPECT_EQ(created_buffers[0]->bound_base_points[0], 4u);
  ASSERT_EQ(created_buffers[1]->bound_base_points.size(), 1u);
  EXPECT_EQ(created_buffers[1]->bound_base_points[0], 9u);
}

TEST(rhi_logic_test, DirectBindBaseDoesNotCountAsDescriptorApplyBufferBindings) {
  cg::test::ScopedFakeDevice scoped_device;

  std::unique_ptr<cg::rhi::Buffer> storage_buffer = cg::rhi::GetDevice().CreateBuffer(cg::rhi::BufferType::kStorage);
  ASSERT_TRUE(storage_buffer != nullptr);

  storage_buffer->BindBase(6);

  const cg::test::FakeBufferBindingState& buffer_binding_state = scoped_device.device().buffer_binding_state();
  EXPECT_EQ(buffer_binding_state.apply_call_count, 0);
  EXPECT_TRUE(buffer_binding_state.calls.empty());

  const auto& created_buffers = scoped_device.device().created_buffers();
  ASSERT_EQ(created_buffers.size(), 1u);
  ASSERT_EQ(created_buffers[0]->bound_base_points.size(), 1u);
  EXPECT_EQ(created_buffers[0]->bound_base_points[0], 6u);
}

TEST(rhi_logic_test, ScopedRenderPassBeginsAndEndsPassWithinLifetime) {
  cg::test::ScopedFakeDevice scoped_device;

  cg::rhi::RenderPassDesc render_pass_desc;
  render_pass_desc.framebuffer = 9;
  render_pass_desc.viewport_size = {16, 8};

  {
    cg::rhi::ScopedRenderPass scoped_render_pass(cg::rhi::GetDevice(), render_pass_desc);
    const cg::test::FakeRenderPassState& render_pass_state = scoped_device.device().render_pass_state();
    EXPECT_EQ(render_pass_state.begin_call_count, 1);
    EXPECT_EQ(render_pass_state.end_call_count, 0);
    EXPECT_EQ(render_pass_state.last_desc.framebuffer, 9u);
  }

  const cg::test::FakeRenderPassState& render_pass_state = scoped_device.device().render_pass_state();
  EXPECT_EQ(render_pass_state.begin_call_count, 1);
  EXPECT_EQ(render_pass_state.end_call_count, 1);
}

TEST(rhi_logic_test, ScopedFramebufferStateCapturesAndRestoresStateWithinLifetime) {
  cg::test::ScopedFakeDevice scoped_device;

  cg::rhi::GetDevice().RestoreFramebufferState({
      .read_framebuffer = 3,
      .draw_framebuffer = 4,
      .framebuffer = 4,
      .viewport = {5, 6, 7, 8},
  });

  {
    cg::rhi::ScopedFramebufferState scoped_state(cg::rhi::GetDevice());
    const cg::rhi::FramebufferState captured = scoped_state.captured_state();
    EXPECT_EQ(captured.read_framebuffer, 3);
    EXPECT_EQ(captured.draw_framebuffer, 4);
    EXPECT_EQ(captured.framebuffer, 4);
    EXPECT_EQ(captured.viewport, glm::ivec4(5, 6, 7, 8));

    cg::rhi::GetDevice().RestoreFramebufferState({
        .read_framebuffer = 9,
        .draw_framebuffer = 10,
        .framebuffer = 10,
        .viewport = {11, 12, 13, 14},
    });
  }

  const cg::rhi::FramebufferState restored = cg::rhi::GetDevice().CaptureFramebufferState();
  EXPECT_EQ(restored.read_framebuffer, 3);
  EXPECT_EQ(restored.draw_framebuffer, 4);
  EXPECT_EQ(restored.framebuffer, 4);
  EXPECT_EQ(restored.viewport, glm::ivec4(5, 6, 7, 8));
}

TEST(rhi_logic_test, ScopedRenderStateCapturesAndRestoresStateWithinLifetime) {
  cg::test::ScopedFakeDevice scoped_device;

  cg::rhi::GetDevice().ApplyRenderState({
      .depth_test_enabled = true,
      .cull_enabled = true,
      .cull_mode = cg::rhi::CullMode::kFront,
      .front_face = cg::rhi::FrontFace::kClockwise,
  });

  {
    cg::rhi::ScopedRenderState scoped_state(cg::rhi::GetDevice());
    const cg::rhi::RenderState captured = scoped_state.captured_state();
    EXPECT_TRUE(captured.depth_test_enabled);
    EXPECT_TRUE(captured.cull_enabled);
    EXPECT_EQ(captured.cull_mode, cg::rhi::CullMode::kFront);
    EXPECT_EQ(captured.front_face, cg::rhi::FrontFace::kClockwise);

    cg::rhi::GetDevice().ApplyRenderState({
        .depth_test_enabled = false,
        .cull_enabled = false,
        .cull_mode = cg::rhi::CullMode::kBack,
        .front_face = cg::rhi::FrontFace::kCounterClockwise,
    });
  }

  const cg::rhi::RenderState restored = cg::rhi::GetDevice().CaptureRenderState();
  EXPECT_TRUE(restored.depth_test_enabled);
  EXPECT_TRUE(restored.cull_enabled);
  EXPECT_EQ(restored.cull_mode, cg::rhi::CullMode::kFront);
  EXPECT_EQ(restored.front_face, cg::rhi::FrontFace::kClockwise);
}

TEST(rhi_logic_test, ProgramDescFlowsThroughActiveDeviceAsExplicitContract) {
  cg::test::ScopedFakeDevice scoped_device;

  const cg::rhi::ProgramDesc render_desc{
      .name = "render_desc",
      .kind = cg::rhi::ProgramKind::kRender,
      .vs = {cg::rhi::ShaderCodePart{.glsl_path = "shader.vert", .code = "void main() {}"}},
      .fs = {cg::rhi::ShaderCodePart{.glsl_path = "shader.frag", .code = "void main() {}"}},
  };
  const cg::rhi::ProgramDesc compute_desc{
      .name = "compute_desc",
      .kind = cg::rhi::ProgramKind::kCompute,
      .cs = {cg::rhi::ShaderCodePart{.glsl_path = "shader.comp", .code = "void main() {}"}},
  };

  ASSERT_TRUE(cg::rhi::GetDevice().CreateProgram(render_desc) != nullptr);
  ASSERT_TRUE(cg::rhi::GetDevice().CreateProgram(compute_desc) != nullptr);

  const auto& created_programs = scoped_device.device().created_programs();
  ASSERT_EQ(created_programs.size(), 2u);
  EXPECT_EQ(created_programs[0]->create_desc.name, render_desc.name);
  EXPECT_EQ(created_programs[0]->create_desc.kind, render_desc.kind);
  ASSERT_EQ(created_programs[0]->create_desc.vs.size(), 1u);
  EXPECT_EQ(created_programs[0]->create_desc.vs.front().glsl_path, "shader.vert");
  EXPECT_EQ(created_programs[1]->create_desc.name, compute_desc.name);
  EXPECT_EQ(created_programs[1]->create_desc.kind, compute_desc.kind);
  ASSERT_EQ(created_programs[1]->create_desc.cs.size(), 1u);
  EXPECT_EQ(created_programs[1]->create_desc.cs.front().glsl_path, "shader.comp");
}

TEST(rhi_logic_test, OpenGLTypeConversionsMapToExpectedConstants) {
  EXPECT_EQ(cg::rhi::ToGLPrimitiveTopology(cg::rhi::PrimitiveTopology::kTriangles), GL_TRIANGLES);
  EXPECT_EQ(cg::rhi::ToGLPrimitiveTopology(cg::rhi::PrimitiveTopology::kLineStrip), GL_LINE_STRIP);
  EXPECT_EQ(cg::rhi::ToGLTextureFormat(cg::rhi::TextureFormat::kRGBA32F), GL_RGBA32F);
  EXPECT_EQ(cg::rhi::ToGLTextureFormat(cg::rhi::TextureFormat::kStencil8), GL_STENCIL_INDEX8);
  EXPECT_EQ(cg::rhi::ToGLPixelFormat(cg::rhi::PixelFormat::kRedInteger), GL_RED_INTEGER);
  EXPECT_EQ(cg::rhi::ToGLPixelFormat(cg::rhi::PixelFormat::kStencilIndex), GL_STENCIL_INDEX);
  EXPECT_EQ(cg::rhi::ToGLPixelType(cg::rhi::PixelType::kUInt32), GL_UNSIGNED_INT);
  EXPECT_EQ(cg::rhi::ToGLFilterMode(cg::rhi::FilterMode::kLinearMipmapLinear), GL_LINEAR_MIPMAP_LINEAR);
  EXPECT_EQ(cg::rhi::ToGLWrapMode(cg::rhi::WrapMode::kClampToEdge), GL_CLAMP_TO_EDGE);
  EXPECT_EQ(cg::rhi::ToGLCullMode(cg::rhi::CullMode::kFrontAndBack), GL_FRONT_AND_BACK);
  EXPECT_EQ(cg::rhi::ToGLFrontFace(cg::rhi::FrontFace::kCounterClockwise), GL_CCW);
  EXPECT_EQ(cg::rhi::ToGLClearMask(ClearMask::kColor | ClearMask::kStencil),
            static_cast<GLbitfield>(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
}

TEST(rhi_logic_test, SetDeviceExposesCapabilitiesAndSupportsReplacement) {
  cg::rhi::Capabilities initial_capabilities = cg::test::FakeDevice::DefaultCapabilities();
  initial_capabilities.backend = BackendType::kOpenGL;
  initial_capabilities.graphics_api_name = "Initial API";

  auto initial_device = std::make_unique<cg::test::FakeDevice>(initial_capabilities);
  cg::test::FakeDevice* initial_device_ptr = initial_device.get();
  cg::rhi::SetDevice(std::move(initial_device));

  ASSERT_TRUE(cg::rhi::HasDevice());
  EXPECT_EQ(&cg::rhi::GetDevice(), initial_device_ptr);
  EXPECT_EQ(cg::rhi::GetCapabilities().graphics_api_name, "Initial API");

  cg::rhi::Capabilities replacement_capabilities = cg::test::FakeDevice::DefaultCapabilities();
  replacement_capabilities.backend = BackendType::kVulkan;
  replacement_capabilities.graphics_api_name = "Replacement API";
  replacement_capabilities.shader_language_name = "SPIR-V";

  auto replacement_device = std::make_unique<cg::test::FakeDevice>(replacement_capabilities);
  cg::test::FakeDevice* replacement_device_ptr = replacement_device.get();
  cg::rhi::SetDevice(std::move(replacement_device));

  ASSERT_TRUE(cg::rhi::HasDevice());
  EXPECT_EQ(&cg::rhi::GetDevice(), replacement_device_ptr);
  EXPECT_EQ(cg::rhi::GetCapabilities().backend, BackendType::kVulkan);
  EXPECT_EQ(cg::rhi::GetCapabilities().graphics_api_name, "Replacement API");
  EXPECT_EQ(cg::rhi::GetCapabilities().shader_language_name, "SPIR-V");

  cg::rhi::SetDevice(std::unique_ptr<cg::rhi::Device>());
  EXPECT_FALSE(cg::rhi::HasDevice());
}

TEST(rhi_logic_test, SceneBackendHelpersPreferSceneOverridesWhenAvailable) {
  cg::rhi::Capabilities capabilities = cg::test::FakeDevice::DefaultCapabilities();
  capabilities.backend = BackendType::kVulkan;
  capabilities.graphics_api_name = "Vulkan 1.3";
  capabilities.scene_backend = BackendType::kOpenGL;
  capabilities.scene_api_name = "OpenGL 4.5 (llvmpipe)";

  cg::test::ScopedFakeDevice scoped_device(capabilities);

  EXPECT_EQ(cg::rhi::GetSceneBackendType(), BackendType::kOpenGL);
  EXPECT_EQ(cg::rhi::GetSceneApiName(), "OpenGL 4.5 (llvmpipe)");
}

TEST(rhi_logic_test, SceneBackendHelpersFallBackToRuntimeBackendWhenSceneOverridesMissing) {
  cg::rhi::Capabilities capabilities = cg::test::FakeDevice::DefaultCapabilities();
  capabilities.backend = BackendType::kVulkan;
  capabilities.graphics_api_name = "Vulkan 1.3";
  capabilities.scene_backend = BackendType::kUnknown;
  capabilities.scene_api_name.clear();

  cg::test::ScopedFakeDevice scoped_device(capabilities);

  EXPECT_EQ(cg::rhi::GetSceneBackendType(), BackendType::kVulkan);
  EXPECT_EQ(cg::rhi::GetSceneApiName(), "Vulkan 1.3");
}

}  // namespace
