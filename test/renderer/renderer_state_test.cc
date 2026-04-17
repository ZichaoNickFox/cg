#include "gtest/gtest.h"

#include <algorithm>
#include <array>
#include <limits>
#include <string>

#include "glm/glm.hpp"

#include "renderer/framebuffer_attachment.h"
#include "renderer/io.h"
#include "renderer/material.h"
#include "renderer/texture.h"
#include "test/test_support/fake_renderer_device.h"

namespace {

void ExpectVec2Near(const glm::vec2& actual, const glm::vec2& expected, float epsilon = 1e-6f) {
  EXPECT_NEAR(actual.x, expected.x, epsilon);
  EXPECT_NEAR(actual.y, expected.y, epsilon);
}

void ExpectVec4Near(const glm::vec4& actual, const glm::vec4& expected, float epsilon = 1e-6f) {
  EXPECT_NEAR(actual.x, expected.x, epsilon);
  EXPECT_NEAR(actual.y, expected.y, epsilon);
  EXPECT_NEAR(actual.z, expected.z, epsilon);
  EXPECT_NEAR(actual.w, expected.w, epsilon);
}

TEST(renderer_state_test, TextureMetaAndTextureExposeResolutionAndDataSizing) {
  cg::Texture::Meta meta;
  meta.type = cg::Texture::kTexture2DArray;
  meta.width = 8;
  meta.height = 4;
  meta.channel_num = 4;
  meta.hdr = false;
  meta.level_num = 1;
  meta.depth = 3;
  meta.format = cg::rhi::TextureFormat::kRGBA8;
  meta.pixel_format = cg::rhi::PixelFormat::kRGBA;
  meta.pixel_type = cg::rhi::PixelType::kUInt8;
  meta.min_filter = cg::rhi::FilterMode::kLinear;
  meta.mag_filter = cg::rhi::FilterMode::kNearest;
  meta.wrap_s = cg::rhi::WrapMode::kClampToEdge;
  meta.wrap_t = cg::rhi::WrapMode::kRepeat;

  EXPECT_TRUE(meta.Varify());
  ExpectVec2Near(meta.Resolution(), glm::vec2(8.0f, 4.0f));
  EXPECT_EQ(meta.data_size_in_byte(), 8 * 4 * 3 * 4);
  EXPECT_EQ(meta.resized_data_size_in_byte(2, 6), 2 * 6 * 3 * 4);

  cg::Texture texture(42u, meta, "g-buffer");
  EXPECT_FALSE(texture.empty());
  EXPECT_EQ(texture.id(), 42u);
  EXPECT_EQ(texture.info(), "g-buffer");
  EXPECT_EQ(texture.data_size_in_byte(), meta.data_size_in_byte());
  EXPECT_TRUE(texture.Varify());

  texture.SetInfo("composited");
  EXPECT_EQ(texture.info(), "composited");
}

TEST(renderer_state_test, FramebufferAttachmentHelpersDescribeExpectedDefaults) {
  const cg::Texture::Meta meta =
      cg::MakeFramebufferTextureMeta(2,
                                     cg::rhi::TextureFormat::kRG32F,
                                     cg::rhi::PixelFormat::kRG,
                                     cg::rhi::PixelType::kFloat32);

  EXPECT_EQ(meta.type, cg::Texture::kTexture2D);
  EXPECT_EQ(meta.width, -1);
  EXPECT_EQ(meta.height, -1);
  EXPECT_EQ(meta.channel_num, 2);
  EXPECT_TRUE(meta.hdr);
  EXPECT_EQ(meta.level_num, 1);
  EXPECT_EQ(meta.depth, 1);
  EXPECT_EQ(meta.min_filter, cg::rhi::FilterMode::kNearest);
  EXPECT_EQ(meta.mag_filter, cg::rhi::FilterMode::kNearest);
  EXPECT_EQ(meta.wrap_s, cg::rhi::WrapMode::kClampToBorder);
  EXPECT_EQ(meta.wrap_t, cg::rhi::WrapMode::kClampToBorder);

  EXPECT_EQ(cg::kAttachmentColor.type, cg::FramebufferAttachment::kColor);
  EXPECT_EQ(cg::kAttachmentColor.clear_type, cg::FramebufferAttachment::kClear);
  EXPECT_EQ(cg::kAttachmentColorNoClear.clear_type, cg::FramebufferAttachment::kNoClear);
  EXPECT_EQ(cg::kAttachmentTexcoord.texture_meta.channel_num, 2);
  EXPECT_EQ(cg::kAttachmentDepth.type, cg::FramebufferAttachment::kDepth);
  EXPECT_EQ(cg::kAttachmentPrimitiveIndex.texture_meta.format, cg::rhi::TextureFormat::kR32UI);
  EXPECT_EQ(cg::kAttachmentPrimitiveIndex.texture_meta.pixel_format, cg::rhi::PixelFormat::kRedInteger);
}

TEST(renderer_state_test, TextureDelegatesUploadReadbackAndReleaseThroughRhiDevice) {
  cg::test::ScopedFakeRendererDevice scoped_device;

  cg::Texture::Meta meta;
  meta.type = cg::Texture::kTexture2D;
  meta.width = 2;
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
  const std::array<uint8_t, 8> pixels = {10u, 20u, 30u, 40u, 50u, 60u, 70u, 80u};
  texture.SetCpuLevelData(0, pixels.data(), pixels.size());

  const uint32_t texture_id = texture.id();
  EXPECT_NE(texture_id, std::numeric_limits<uint32_t>::max());
  ASSERT_TRUE(texture.storage() != nullptr);
  EXPECT_TRUE(texture.storage()->uploaded_to_gl);

  const std::vector<unsigned char> readback = texture.GetData<unsigned char>();
  EXPECT_EQ(readback.size(), pixels.size());
  EXPECT_TRUE(std::equal(readback.begin(), readback.end(), pixels.begin(), pixels.end()));

  texture.ReleaseGpuResources();
  EXPECT_FALSE(texture.storage()->uploaded_to_gl);
  EXPECT_EQ(texture.storage()->id, std::numeric_limits<uint32_t>::max());
}

TEST(renderer_state_test, IoTracksCursorButtonsKeysAndClipboardState) {
  cg::Io io;
  io.SetScreenSize({200, 100});
  io.SetFramebufferSize({400, 200});

  EXPECT_EQ(io.screen_size(), glm::ivec2(200, 100));
  EXPECT_EQ(io.framebuffer_size(), glm::ivec2(400, 200));

  io.FeedKeyInput("W");
  io.FeedKeyInput("Space");
  EXPECT_TRUE(io.HadKeyInput("W"));
  EXPECT_TRUE(io.HadKeyInput("Space"));
  io.ClearKeyInput();
  EXPECT_FALSE(io.HadKeyInput("W"));

  io.FeedCursorPos({50.0f, 25.0f});
  ExpectVec2Near(io.GetCursorDelta(), glm::vec2(50.0f, 25.0f));
  io.FeedCursorPos({120.0f, 75.0f});
  ExpectVec2Near(io.GetCursorDelta(), glm::vec2(70.0f, 50.0f));
  ExpectVec2Near(io.GetCursorWindowPos(), glm::vec2(120.0f, 25.0f));
  ExpectVec2Near(io.GetCursorPosSS(), glm::vec2(0.6f, 0.25f));

  io.FeedButtonInput(true, false);
  EXPECT_TRUE(io.left_button_pressed());
  EXPECT_FALSE(io.right_button_pressed());

  io.SetGuiCapturedMouse(true);
  EXPECT_TRUE(io.gui_captured_cursor());

  std::string clipboard_text;
  io.SetWriteClipboardFunc([&clipboard_text](const std::string& text) { clipboard_text = text; });
  io.write_clipboard_func()("copied");
  EXPECT_EQ(clipboard_text, "copied");
}

TEST(renderer_state_test, MaterialPropertiesPopulateSupportedFieldsAndPreserveDefaults) {
  const cg::Material material({
      {cg::kAlbedo, glm::vec4(0.1f, 0.2f, 0.3f, 1.0f)},
      {cg::kEmission, glm::vec4(0.4f, 0.5f, 0.6f, 1.0f)},
      {cg::kRoughness, 0.25},
      {cg::kMetallic, 0.75},
      {cg::kShininess, 32.0},
      {cg::kTextureNormal, 3},
      {cg::kTextureDiffuse, 5},
      {cg::kTextureBaseColor, 7},
      {cg::kTextureDiffuseRoughness, 11},
      {cg::kTextureMetalness, 13},
      {cg::kTextureAmbientOcclusion, 17},
      {cg::kTextureHeight, 19},
      {cg::kTextureShininess, 23},
  });

  ExpectVec4Near(material.albedo, glm::vec4(0.1f, 0.2f, 0.3f, 1.0f));
  ExpectVec4Near(material.emission, glm::vec4(0.4f, 0.5f, 0.6f, 1.0f));
  ExpectVec4Near(material.ambient, glm::vec4(0.0f));
  EXPECT_FLOAT_EQ(material.roughness, 0.25f);
  EXPECT_FLOAT_EQ(material.metalness, 0.75f);
  EXPECT_FLOAT_EQ(material.shininess, 32.0f);
  EXPECT_EQ(material.texture_normal, 3);
  EXPECT_EQ(material.texture_diffuse, 5);
  EXPECT_EQ(material.texture_base_color, 7);
  EXPECT_EQ(material.texture_roughness, 11);
  EXPECT_EQ(material.texture_metalness, 13);
  EXPECT_EQ(material.texture_ambient_occlusion, 17);
  EXPECT_EQ(material.texture_height, 19);
  EXPECT_EQ(material.texture_shininess, 23);
  EXPECT_EQ(material.texture_specular, -1);
}

}  // namespace
