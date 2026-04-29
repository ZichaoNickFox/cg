#include "gtest/gtest.h"

#include <array>
#include <cstdint>

#include "glm/glm.hpp"

#include "renderer/automic_counter.h"
#include "renderer/definition.h"
#include "renderer/material.h"
#include "renderer/ssbo.h"
#include "renderer/texture_buffer.h"
#include "test/test_support/fake_device.h"

namespace {

void ExpectVec4Near(const glm::vec4& actual, const glm::vec4& expected, float epsilon = 1e-6f) {
  EXPECT_NEAR(actual.x, expected.x, epsilon);
  EXPECT_NEAR(actual.y, expected.y, epsilon);
  EXPECT_NEAR(actual.z, expected.z, epsilon);
  EXPECT_NEAR(actual.w, expected.w, epsilon);
}

struct Payload {
  int count = 0;
  float scale = 0.0f;
};

TEST(renderer_buffer_test, SSBOUploadsDataBindsOnceAndCanReadBackThroughExplicitBufferReadback) {
  cg::test::ScopedFakeDevice scoped_device;

  cg::SSBO ssbo(7);
  const Payload first{42, 1.5f};
  ssbo.SetData(sizeof(first), &first);

  ASSERT_EQ(scoped_device.device().created_buffers().size(), 1u);
  const auto& buffer = scoped_device.device().created_buffers().front();
  EXPECT_EQ(buffer->type, cg::rhi::BufferType::kStorage);
  EXPECT_EQ(buffer->last_usage, cg::rhi::BufferUsage::kStreamCopy);
  EXPECT_EQ(buffer->set_data_call_count, 1);
  EXPECT_EQ(buffer->bind_base_call_count, 1);
  EXPECT_EQ(scoped_device.device().buffer_binding_state().apply_call_count, 1);
  ASSERT_EQ(scoped_device.device().buffer_binding_state().calls.size(), 1u);
  EXPECT_EQ(scoped_device.device().buffer_binding_state().calls.front().buffer_type, cg::rhi::BufferType::kStorage);
  EXPECT_EQ(scoped_device.device().buffer_binding_state().calls.front().binding_point, 7u);
  ASSERT_EQ(buffer->bound_base_points.size(), 1u);
  EXPECT_EQ(buffer->bound_base_points.front(), 7u);

  const Payload mapped = ssbo.GetData<Payload>();
  EXPECT_EQ(mapped.count, 42);
  EXPECT_FLOAT_EQ(mapped.scale, 1.5f);
  EXPECT_EQ(buffer->read_data_call_count, 1);

  const Payload second{7, 3.25f};
  ssbo.SetData(sizeof(second), &second);
  EXPECT_EQ(buffer->set_data_call_count, 2);
  EXPECT_EQ(buffer->bind_base_call_count, 1);
  EXPECT_EQ(scoped_device.device().buffer_binding_state().apply_call_count, 1);
}

TEST(renderer_buffer_test, TextureBufferUploadsStaticDataThroughTextureBufferBinding) {
  cg::test::ScopedFakeDevice scoped_device;

  cg::TextureBuffer texture_buffer;
  std::array<uint32_t, 4> data = {1u, 2u, 3u, 4u};
  texture_buffer.SetData(sizeof(data), data.data());

  ASSERT_EQ(scoped_device.device().created_buffers().size(), 1u);
  const auto& buffer = scoped_device.device().created_buffers().front();
  EXPECT_EQ(buffer->type, cg::rhi::BufferType::kTexture);
  EXPECT_EQ(buffer->last_usage, cg::rhi::BufferUsage::kStatic);
  EXPECT_EQ(buffer->set_data_call_count, 1);
  ASSERT_EQ(buffer->data.size(), sizeof(data));
  EXPECT_EQ(cg::test::ReadScalar<uint32_t>(buffer->data, 0), 1u);
  EXPECT_EQ(cg::test::ReadScalar<uint32_t>(buffer->data, sizeof(uint32_t) * 3), 4u);
}

TEST(renderer_buffer_test, AutomicCounterInitializesBindingAndResetsStoredValue) {
  cg::test::ScopedFakeDevice scoped_device;

  cg::AutomicCounter counter;
  ASSERT_EQ(scoped_device.device().created_buffers().size(), 1u);
  const auto& buffer = scoped_device.device().created_buffers().front();
  EXPECT_EQ(buffer->type, cg::rhi::BufferType::kAtomicCounter);

  counter.Init(5);
  EXPECT_EQ(counter.binding_point(), 5);
  EXPECT_EQ(buffer->set_data_call_count, 1);
  EXPECT_EQ(buffer->last_usage, cg::rhi::BufferUsage::kDynamic);
  EXPECT_EQ(scoped_device.device().buffer_binding_state().apply_call_count, 1);
  ASSERT_EQ(scoped_device.device().buffer_binding_state().calls.size(), 1u);
  EXPECT_EQ(scoped_device.device().buffer_binding_state().calls.front().buffer_type,
            cg::rhi::BufferType::kAtomicCounter);
  EXPECT_EQ(scoped_device.device().buffer_binding_state().calls.front().binding_point, 5u);
  ASSERT_EQ(buffer->bound_base_points.size(), 1u);
  EXPECT_EQ(buffer->bound_base_points.front(), 5u);

  counter.Reset(123u);
  EXPECT_EQ(buffer->update_data_call_count, 1);
  EXPECT_EQ(cg::test::ReadScalar<uint32_t>(buffer->data), 123u);
}

TEST(renderer_buffer_test, MaterialRepoUploadsOnlyWhenDirtyAndMaintainsLookups) {
  cg::test::ScopedFakeDevice scoped_device;

  cg::MaterialRepo repo;
  ASSERT_EQ(scoped_device.device().created_buffers().size(), 1u);
  const auto& buffer = scoped_device.device().created_buffers().front();
  EXPECT_EQ(buffer->type, cg::rhi::BufferType::kStorage);
  EXPECT_EQ(repo.num(), 4);
  EXPECT_TRUE(repo.Has("gold"));

  const int gold_index = repo.GetMaterialIndex("gold");
  EXPECT_EQ(repo.GetName(gold_index), "gold");

  repo.UpdateSSBO();
  EXPECT_EQ(buffer->set_data_call_count, 1);
  EXPECT_EQ(buffer->bind_base_call_count, 1);
  EXPECT_EQ(scoped_device.device().buffer_binding_state().apply_call_count, 1);
  EXPECT_EQ(buffer->last_usage, cg::rhi::BufferUsage::kStreamCopy);

  repo.UpdateSSBO();
  EXPECT_EQ(buffer->set_data_call_count, 1);

  repo.mutable_material(gold_index)->roughness = 0.5f;
  repo.UpdateSSBO();
  EXPECT_EQ(buffer->set_data_call_count, 2);

  repo.Add("custom",
           cg::Material({
               {cg::kEmission, glm::vec4(1.0f, 0.5f, 0.25f, 1.0f)},
               {cg::kTextureDiffuse, 9},
           }));
  EXPECT_TRUE(repo.Has("custom"));
  EXPECT_EQ(repo.GetName(repo.GetIndex("custom")), "custom");
  ExpectVec4Near(repo.GetMaterial("custom").emission, glm::vec4(1.0f, 0.5f, 0.25f, 1.0f));
  EXPECT_EQ(repo.GetMaterial("custom").texture_diffuse, 9);

  repo.UpdateSSBO();
  EXPECT_EQ(buffer->set_data_call_count, 3);
}

}  // namespace
