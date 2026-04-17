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

  ASSERT_EQ(scoped_device.device().draw_calls().size(), 1u);
  EXPECT_EQ(scoped_device.device().draw_calls().front().kind, cg::test::FakeDrawCall::Kind::kElements);
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
  EXPECT_EQ(scoped_device.device().draw_calls().front().kind, cg::test::FakeDrawCall::Kind::kArrays);
  EXPECT_EQ(scoped_device.device().draw_calls().front().topology, cg::rhi::PrimitiveTopology::kLineStrip);
  EXPECT_EQ(scoped_device.device().draw_calls().front().count, 4u);
  EXPECT_EQ(scoped_device.device().draw_calls().front().instance_count, 5u);
}

}  // namespace
