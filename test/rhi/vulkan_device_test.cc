#include "gtest/gtest.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <memory>
#include <vector>

#include "renderer/texture.h"
#include "rhi/device.h"

namespace {

class ScopedDeviceReset {
 public:
  ~ScopedDeviceReset() {
    cg::rhi::SetDevice(std::unique_ptr<cg::rhi::Device>());
  }
};

TEST(vulkan_device_test, PresentationOnlyDeviceSupportsSyntheticTextureLifecycle) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  EXPECT_EQ(device->capabilities().backend, cg::rhi::BackendType::kVulkan);
  EXPECT_EQ(device->capabilities().scene_backend, cg::rhi::BackendType::kUnknown);
  EXPECT_EQ(device->capabilities().scene_api_name, "Unavailable (presentation-only Vulkan RHI)");

  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::Texture::Meta meta;
  meta.type = cg::Texture::kTexture2D;
  meta.width = 1;
  meta.height = 1;
  meta.channel_num = 4;
  meta.hdr = false;
  meta.level_num = 1;
  meta.depth = 1;
  meta.format = cg::rhi::TextureFormat::kRGBA8;
  meta.pixel_format = cg::rhi::PixelFormat::kRGBA;
  meta.pixel_type = cg::rhi::PixelType::kUInt8;
  meta.min_filter = cg::rhi::FilterMode::kLinear;
  meta.mag_filter = cg::rhi::FilterMode::kLinear;
  meta.wrap_s = cg::rhi::WrapMode::kRepeat;
  meta.wrap_t = cg::rhi::WrapMode::kRepeat;

  cg::Texture texture(meta);
  const std::array<uint8_t, 4> pixels = {1u, 2u, 3u, 4u};
  texture.SetCpuLevelData(0, pixels.data(), pixels.size());

  const uint32_t texture_id = texture.id();
  EXPECT_NE(texture_id, std::numeric_limits<uint32_t>::max());
  ASSERT_TRUE(texture.storage() != nullptr);
  EXPECT_TRUE(texture.storage()->uploaded_to_gl);

  const std::vector<unsigned char> readback = texture.GetData<unsigned char>();
  EXPECT_EQ(readback.size(), pixels.size());
  EXPECT_TRUE(std::equal(readback.begin(), readback.end(), pixels.begin(), pixels.end()));

  const std::shared_ptr<cg::rhi::Program> program =
      cg::rhi::GetDevice().CreateRenderProgram("synthetic", {}, {}, {}, {});
  ASSERT_TRUE(program != nullptr);
  EXPECT_EQ(program->BindTexture("albedo", texture), 0);
  EXPECT_EQ(program->BindTexture("albedo", texture), 0);

  texture.ReleaseGpuResources();
  EXPECT_FALSE(texture.storage()->uploaded_to_gl);
  EXPECT_EQ(texture.storage()->id, std::numeric_limits<uint32_t>::max());
}

}  // namespace
