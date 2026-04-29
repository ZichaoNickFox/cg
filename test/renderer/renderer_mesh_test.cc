#include "gtest/gtest.h"

#include <vector>

#include "glm/glm.hpp"

#include "renderer/mesh.h"
#include "renderer/mesh/lines_mesh.h"
#include "test/test_support/fake_device.h"

namespace {

TEST(renderer_mesh_test, MeshSubmitUploadsGeometryOnceAndReusesGpuStateUntilDirty) {
  cg::test::ScopedFakeDevice scoped_device;

  cg::Mesh mesh;
  mesh.SetPositions({
      glm::vec3(-1.0f, 0.0f, 0.0f),
      glm::vec3(1.0f, 0.0f, 0.0f),
      glm::vec3(0.0f, 1.0f, 0.0f),
  });
  mesh.SetIndices({0u, 1u, 2u});

  mesh.Submit();

  ASSERT_EQ(scoped_device.device().created_vertex_arrays().size(), 1u);
  ASSERT_EQ(scoped_device.device().created_buffers().size(), 2u);

  const auto& vertex_array = scoped_device.device().created_vertex_arrays().front();
  EXPECT_EQ(vertex_array->apply_binding_call_count, 1);
  ASSERT_EQ(vertex_array->binding_descs.size(), 1u);
  ASSERT_EQ(vertex_array->binding_descs.front().attributes.size(), 1u);
  ASSERT_TRUE(vertex_array->attributes.contains(0u));
  EXPECT_EQ(vertex_array->attributes.at(0u).component_count, 3);
  EXPECT_EQ(vertex_array->attributes.at(0u).stride_in_bytes, static_cast<int>(sizeof(glm::vec3)));
  EXPECT_EQ(vertex_array->attributes.at(0u).offset_in_bytes, 0u);

  const auto& vertex_buffer = scoped_device.device().created_buffers().front();
  const auto& index_buffer = scoped_device.device().created_buffers().back();
  EXPECT_EQ(vertex_buffer->type, cg::rhi::BufferType::kVertex);
  EXPECT_EQ(index_buffer->type, cg::rhi::BufferType::kIndex);
  EXPECT_EQ(vertex_buffer->set_data_call_count, 1);
  EXPECT_EQ(index_buffer->set_data_call_count, 1);
  EXPECT_EQ(vertex_buffer->bind_call_count, 0);
  EXPECT_EQ(vertex_buffer->unbind_call_count, 0);

  ASSERT_EQ(scoped_device.device().draw_calls().size(), 1u);
  EXPECT_EQ(scoped_device.device().draw_calls().front().kind, cg::rhi::DrawKind::kElements);
  EXPECT_TRUE(scoped_device.device().draw_calls().front().has_vertex_array);
  EXPECT_TRUE(scoped_device.device().draw_calls().front().has_index_buffer);
  EXPECT_EQ(scoped_device.device().draw_calls().front().count, 3u);
  EXPECT_EQ(scoped_device.device().draw_calls().front().instance_count, 1u);

  mesh.Submit();
  EXPECT_EQ(scoped_device.device().created_vertex_arrays().size(), 1u);
  EXPECT_EQ(scoped_device.device().created_buffers().size(), 2u);
  ASSERT_EQ(scoped_device.device().draw_calls().size(), 2u);

  mesh.SetPositions({
      glm::vec3(-2.0f, 0.0f, 0.0f),
      glm::vec3(2.0f, 0.0f, 0.0f),
      glm::vec3(0.0f, 2.0f, 0.0f),
  });
  mesh.Submit();
  EXPECT_EQ(scoped_device.device().created_vertex_arrays().size(), 2u);
  EXPECT_EQ(scoped_device.device().created_buffers().size(), 4u);
  ASSERT_EQ(scoped_device.device().draw_calls().size(), 3u);
}

TEST(renderer_mesh_test, MeshBuildDrawDescPreparesReusableGeometryDescriptorWithoutSubmittingDraw) {
  cg::test::ScopedFakeDevice scoped_device;

  cg::Mesh mesh;
  mesh.SetPositions({
      glm::vec3(-1.0f, 0.0f, 0.0f),
      glm::vec3(1.0f, 0.0f, 0.0f),
      glm::vec3(0.0f, 1.0f, 0.0f),
  });
  mesh.SetIndices({0u, 1u, 2u});

  const cg::rhi::DrawDesc first_desc = mesh.BuildDrawDesc(4);
  EXPECT_EQ(first_desc.kind, cg::rhi::DrawKind::kElements);
  EXPECT_EQ(first_desc.topology, cg::rhi::PrimitiveTopology::kTriangles);
  EXPECT_NE(first_desc.vertex_array, nullptr);
  EXPECT_NE(first_desc.index_buffer, nullptr);
  EXPECT_EQ(first_desc.count, 3u);
  EXPECT_EQ(first_desc.instance_count, 4u);
  EXPECT_TRUE(scoped_device.device().draw_calls().empty());

  const cg::rhi::DrawDesc second_desc = mesh.BuildDrawDesc();
  EXPECT_EQ(second_desc.vertex_array, first_desc.vertex_array);
  EXPECT_EQ(second_desc.index_buffer, first_desc.index_buffer);
  EXPECT_EQ(scoped_device.device().created_vertex_arrays().size(), 1u);
  EXPECT_EQ(scoped_device.device().created_buffers().size(), 2u);
  EXPECT_TRUE(scoped_device.device().draw_calls().empty());
}

TEST(renderer_mesh_test, LinesMeshUsesArrayDrawPathForInstancedSubmission) {
  cg::test::ScopedFakeDevice scoped_device;

  cg::LinesMesh mesh({
      glm::vec3(0.0f, 0.0f, 0.0f),
      glm::vec3(1.0f, 0.0f, 0.0f),
      glm::vec3(1.0f, 1.0f, 0.0f),
      glm::vec3(0.0f, 1.0f, 0.0f),
  }, {
      glm::vec4(1.0f),
      glm::vec4(1.0f),
      glm::vec4(1.0f),
      glm::vec4(1.0f),
  }, cg::rhi::PrimitiveTopology::kLineStrip);

  mesh.Submit(5);

  ASSERT_EQ(scoped_device.device().draw_calls().size(), 1u);
  EXPECT_EQ(scoped_device.device().draw_calls().front().kind, cg::rhi::DrawKind::kArrays);
  EXPECT_EQ(scoped_device.device().draw_calls().front().topology, cg::rhi::PrimitiveTopology::kLineStrip);
  EXPECT_TRUE(scoped_device.device().draw_calls().front().has_vertex_array);
  EXPECT_FALSE(scoped_device.device().draw_calls().front().has_index_buffer);
  EXPECT_EQ(scoped_device.device().draw_calls().front().count, 4u);
  EXPECT_EQ(scoped_device.device().draw_calls().front().instance_count, 5u);
}

}  // namespace
