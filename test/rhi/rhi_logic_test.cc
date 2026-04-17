#include "gtest/gtest.h"

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

TEST(rhi_logic_test, OpenGLTypeConversionsMapToExpectedConstants) {
  EXPECT_EQ(cg::rhi::ToGLPrimitiveTopology(cg::rhi::PrimitiveTopology::kTriangles), GL_TRIANGLES);
  EXPECT_EQ(cg::rhi::ToGLPrimitiveTopology(cg::rhi::PrimitiveTopology::kLineStrip), GL_LINE_STRIP);
  EXPECT_EQ(cg::rhi::ToGLTextureFormat(cg::rhi::TextureFormat::kRGBA32F), GL_RGBA32F);
  EXPECT_EQ(cg::rhi::ToGLPixelFormat(cg::rhi::PixelFormat::kRedInteger), GL_RED_INTEGER);
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
