#include "gtest/gtest.h"

#include <algorithm>
#include <array>
#include <limits>
#include <string>

#include "glm/glm.hpp"

#include "renderer/config.h"
#include "renderer/framebuffer_attachment.h"
#include "renderer/framebuffer.h"
#include "renderer/io.h"
#include "renderer/material.h"
#include "renderer/scene.h"
#include "renderer/shader.h"
#include "renderer/shader_program.h"
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

void ExpectVec3Near(const glm::vec3& actual, const glm::vec3& expected, float epsilon = 1e-6f) {
  EXPECT_NEAR(actual.x, expected.x, epsilon);
  EXPECT_NEAR(actual.y, expected.y, epsilon);
  EXPECT_NEAR(actual.z, expected.z, epsilon);
}

void ExpectMat4Near(const glm::mat4& actual, const glm::mat4& expected, float epsilon = 1e-6f) {
  for (int column = 0; column < 4; ++column) {
    for (int row = 0; row < 4; ++row) {
      EXPECT_NEAR(actual[column][row], expected[column][row], epsilon);
    }
  }
}

cg::Texture::Meta MakeTextureMeta() {
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
  meta.min_filter = cg::rhi::FilterMode::kNearest;
  meta.mag_filter = cg::rhi::FilterMode::kNearest;
  meta.wrap_s = cg::rhi::WrapMode::kClampToEdge;
  meta.wrap_t = cg::rhi::WrapMode::kClampToEdge;
  return meta;
}

cg::Texture MakeSolidTexture(const std::array<uint8_t, 4>& pixel) {
  cg::Texture texture(MakeTextureMeta());
  texture.SetCpuLevelData(0, pixel.data(), pixel.size());
  return texture;
}

class TestScene final : public cg::Scene {
 public:
  void OnEnter() override {}
  void OnUpdate() override {}
  void OnRender() override {}
  void OnExit() override {}

  void AddObject(const cg::ObjectMeta& meta) {
    object_repo_.AddOrReplace(meta);
  }

  void AddTexture(const std::string& name, const cg::Texture& texture) {
    texture_repo_.AddUnique(name, texture);
  }

  const cg::Object& GetObject(const std::string& name) const {
    return object_repo_.GetObject(name);
  }
};

class EnteredTestScene {
 public:
  EnteredTestScene() {
    config_.Init(std::string(CG_PROJECT_SOURCE_DIR) + "/playground/config.pb.txt");
    io_.SetScreenSize({64, 64});
    io_.SetFramebufferSize({64, 64});
    scene_.Enter("renderer_state_test_scene", &config_, &io_, &frame_stat_);
  }

  ~EnteredTestScene() {
    scene_.Exit();
  }

  TestScene& scene() { return scene_; }
  const TestScene& scene() const { return scene_; }

 private:
  cg::Config config_;
  cg::Io io_;
  cg::FrameStat frame_stat_;
  TestScene scene_;
};

class TestCommonRenderShader : public cg::RenderShader {
 public:
  TestCommonRenderShader(const cg::Scene& scene, const cg::Object& object)
      : RenderShader(scene, "sample_scene") {
    cg::ShaderProgramBindings bindings;
    cg::AppendRenderObjectBindings(object, scene.camera(), &bindings);
    DrawBindings(bindings, scene, object);
  }
};

class TestCommonComputeShader : public cg::ComputeShader {
 public:
  explicit TestCommonComputeShader(const cg::Scene& scene)
      : ComputeShader(scene, "path_tracing_scene") {
    cg::ShaderProgramBindings bindings;
    cg::AppendResolutionBindings({64.0f, 32.0f}, &bindings);
    DispatchBindings(bindings, {
        .workgroup_count = glm::uvec3(2, 3, 1),
    });
  }
};

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
  EXPECT_EQ(cg::kAttachmentStencil.type, cg::FramebufferAttachment::kStencil);
  EXPECT_EQ(cg::kAttachmentStencil.texture_meta.format, cg::rhi::TextureFormat::kStencil8);
  EXPECT_EQ(cg::kAttachmentStencil.texture_meta.pixel_format, cg::rhi::PixelFormat::kStencilIndex);
  EXPECT_EQ(cg::kAttachmentStencil.texture_meta.pixel_type, cg::rhi::PixelType::kUInt8);
  EXPECT_FALSE(cg::kAttachmentStencil.texture_meta.hdr);
  EXPECT_EQ(cg::kAttachmentPrimitiveIndex.texture_meta.format, cg::rhi::TextureFormat::kR32UI);
  EXPECT_EQ(cg::kAttachmentPrimitiveIndex.texture_meta.pixel_format, cg::rhi::PixelFormat::kRedInteger);
}

TEST(renderer_state_test, TextureDelegatesUploadReadbackAndReleaseThroughRhiDevice) {
  cg::test::ScopedFakeRendererDevice scoped_device;

  cg::Texture::Meta meta = MakeTextureMeta();
  meta.width = 2;
  meta.mag_filter = cg::rhi::FilterMode::kLinear;
  meta.min_filter = cg::rhi::FilterMode::kLinear;
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
  const cg::test::FakeTextureReadState& texture_read_state = scoped_device.device().texture_read_state();
  EXPECT_EQ(texture_read_state.call_count, 1);
  EXPECT_EQ(texture_read_state.last_desc.level, 0);
  EXPECT_EQ(texture_read_state.last_desc.size_in_bytes, pixels.size());

  texture.ReleaseGpuResources();
  EXPECT_FALSE(texture.storage()->uploaded_to_gl);
  EXPECT_EQ(texture.storage()->id, std::numeric_limits<uint32_t>::max());
}

TEST(renderer_state_test, TextureRepoAsTexture2DArrayReturnsStableCachedReference) {
  cg::test::ScopedFakeRendererDevice scoped_device;

  cg::TextureRepo repo;
  repo.AddUnique("first", MakeSolidTexture({1u, 2u, 3u, 255u}));
  const cg::Texture* first = &repo.AsTexture2DArray(4, 4);
  ASSERT_NE(first, nullptr);
  EXPECT_EQ(first->meta().type, cg::Texture::kTexture2DArray);
  EXPECT_FALSE(first->empty());

  const cg::Texture* second = &repo.AsTexture2DArray(4, 4);
  EXPECT_EQ(second, first);

  repo.AddUnique("second", MakeSolidTexture({7u, 8u, 9u, 255u}));
  const cg::Texture* third = &repo.AsTexture2DArray(4, 4);
  EXPECT_EQ(third, first);
  EXPECT_FALSE(third->empty());
}

TEST(renderer_state_test, SceneAndFramebufferTextureLookupReturnStableOwnedReferences) {
  cg::test::ScopedFakeRendererDevice scoped_device;

  TestScene scene;
  scene.AddTexture("albedo", MakeSolidTexture({3u, 4u, 5u, 255u}));

  const cg::Texture& scene_first = scene.GetTexture("albedo");
  const cg::Texture& scene_second = scene.GetTexture("albedo");
  EXPECT_EQ(&scene_first, &scene_second);
  EXPECT_EQ(&scene_first, &scene.texture_repo().GetTexture("albedo"));

  cg::ShaderProgramBindings scene_bindings;
  scene_bindings.SetTexture("scene_albedo", scene.GetTexture("albedo"));
  ASSERT_EQ(scene_bindings.rhi_bindings().textures.size(), 1u);
  EXPECT_EQ(scene_bindings.rhi_bindings().textures.front().texture, &scene_first);

  cg::Framebuffer framebuffer;
  cg::Framebuffer::Option option;
  option.size = {4, 4};
  option.attachments = {cg::kAttachmentColor, cg::kAttachmentDepth};
  framebuffer.Init(option);

  const cg::Texture& attachment_first = framebuffer.GetTexture("color");
  const cg::Texture& attachment_second = framebuffer.GetTexture("color");
  EXPECT_EQ(&attachment_first, &attachment_second);

  cg::ShaderProgramBindings framebuffer_bindings;
  framebuffer_bindings.SetTexture("framebuffer_color", framebuffer.GetTexture("color"));
  ASSERT_EQ(framebuffer_bindings.rhi_bindings().textures.size(), 1u);
  EXPECT_EQ(framebuffer_bindings.rhi_bindings().textures.front().texture, &attachment_first);
}

TEST(renderer_state_test, FramebufferUsesRenderPassAbstractionForBindAndRestore) {
  cg::test::ScopedFakeRendererDevice scoped_device;

  cg::Framebuffer framebuffer;
  cg::Framebuffer::Option option;
  option.size = {16, 8};
  option.clear_color = glm::vec4(0.2f, 0.3f, 0.4f, 1.0f);
  option.attachments = {cg::kAttachmentColor, cg::kAttachmentDepth};
  framebuffer.Init(option);
  const cg::test::FakeFramebufferConfigState& framebuffer_config_state =
      scoped_device.device().framebuffer_config_state();
  EXPECT_EQ(framebuffer_config_state.configure_call_count, 1);
  EXPECT_EQ(framebuffer_config_state.framebuffer, framebuffer.fbo());
  ASSERT_EQ(framebuffer_config_state.last_desc.attachments.size(), 2u);
  EXPECT_EQ(framebuffer_config_state.last_desc.attachments[0].attachment_type, cg::rhi::AttachmentType::kColor);
  EXPECT_EQ(framebuffer_config_state.last_desc.attachments[0].attachment_index, 0u);
  ASSERT_NE(framebuffer_config_state.last_desc.attachments[0].texture, nullptr);
  EXPECT_EQ(framebuffer_config_state.last_desc.attachments[0].texture->meta().width, 16);
  EXPECT_EQ(framebuffer_config_state.last_desc.attachments[0].texture->meta().height, 8);
  EXPECT_EQ(framebuffer_config_state.last_desc.attachments[1].attachment_type, cg::rhi::AttachmentType::kDepth);
  EXPECT_EQ(framebuffer_config_state.last_desc.attachments[1].attachment_index, 0u);
  ASSERT_NE(framebuffer_config_state.last_desc.attachments[1].texture, nullptr);
  EXPECT_EQ(framebuffer_config_state.last_desc.attachments[1].texture->meta().width, 16);
  EXPECT_EQ(framebuffer_config_state.last_desc.attachments[1].texture->meta().height, 8);

  cg::rhi::GetDevice().RestoreFramebufferState({
      .read_framebuffer = 77,
      .draw_framebuffer = 77,
      .framebuffer = 77,
      .viewport = {3, 4, 5, 6},
  });

  {
    auto framebuffer_pass = framebuffer.BindScoped();
    EXPECT_TRUE(framebuffer_pass.active());

    const cg::test::FakeRenderPassState& render_pass_state = scoped_device.device().render_pass_state();
    EXPECT_EQ(render_pass_state.begin_call_count, 1);
    EXPECT_EQ(render_pass_state.last_desc.framebuffer, framebuffer.fbo());
    EXPECT_EQ(render_pass_state.last_desc.viewport_origin, glm::ivec2(0, 0));
    EXPECT_EQ(render_pass_state.last_desc.viewport_size, glm::ivec2(16, 8));
    EXPECT_TRUE(render_pass_state.last_desc.clear_depth);
    EXPECT_TRUE(render_pass_state.last_desc.clear_stencil);
    EXPECT_FLOAT_EQ(render_pass_state.last_desc.depth_clear_value, 1.0f);
    EXPECT_EQ(render_pass_state.last_desc.stencil_clear_value, 0);
    ASSERT_EQ(render_pass_state.last_desc.color_attachments.size(), 1u);
    EXPECT_EQ(render_pass_state.last_desc.color_attachments[0].attachment_index, 0u);
    EXPECT_TRUE(render_pass_state.last_desc.color_attachments[0].clear);
    ExpectVec4Near(render_pass_state.last_desc.color_attachments[0].clear_color, option.clear_color);
  }

  const cg::test::FakeRenderPassState& render_pass_state = scoped_device.device().render_pass_state();
  EXPECT_EQ(render_pass_state.end_call_count, 1);
  const cg::rhi::FramebufferState resumed_state = cg::rhi::GetDevice().CaptureFramebufferState();
  EXPECT_EQ(resumed_state.read_framebuffer, 77);
  EXPECT_EQ(resumed_state.draw_framebuffer, 77);
  EXPECT_EQ(resumed_state.framebuffer, 77);
  EXPECT_EQ(resumed_state.viewport, glm::ivec4(3, 4, 5, 6));
}

TEST(renderer_state_test, FramebufferScopedBindResetRestoresStateBeforeScopeExit) {
  cg::test::ScopedFakeRendererDevice scoped_device;

  cg::Framebuffer framebuffer;
  cg::Framebuffer::Option option;
  option.size = {8, 4};
  option.attachments = {cg::kAttachmentColor};
  framebuffer.Init(option);

  cg::rhi::GetDevice().RestoreFramebufferState({
      .read_framebuffer = 23,
      .draw_framebuffer = 24,
      .framebuffer = 24,
      .viewport = {5, 6, 7, 8},
  });

  auto framebuffer_pass = framebuffer.BindScoped();
  EXPECT_TRUE(framebuffer_pass.active());
  EXPECT_EQ(scoped_device.device().render_pass_state().begin_call_count, 1);

  framebuffer_pass.Reset();
  EXPECT_FALSE(framebuffer_pass.active());
  EXPECT_EQ(scoped_device.device().render_pass_state().end_call_count, 1);

  const cg::rhi::FramebufferState resumed_state = cg::rhi::GetDevice().CaptureFramebufferState();
  EXPECT_EQ(resumed_state.read_framebuffer, 23);
  EXPECT_EQ(resumed_state.draw_framebuffer, 24);
  EXPECT_EQ(resumed_state.framebuffer, 24);
  EXPECT_EQ(resumed_state.viewport, glm::ivec4(5, 6, 7, 8));
}

TEST(renderer_state_test, FramebufferBlitUsesDescriptorBasedBlitContract) {
  cg::test::ScopedFakeRendererDevice scoped_device;

  cg::Framebuffer source;
  cg::Framebuffer::Option option;
  option.size = {8, 6};
  option.attachments = {cg::kAttachmentColor};
  source.Init(option);

  cg::Framebuffer destination;
  destination.Init(option);

  source.Blit(&destination);

  const cg::test::FakeBlitFramebufferState& state = scoped_device.device().blit_framebuffer_state();
  EXPECT_EQ(state.call_count, 1);
  EXPECT_EQ(state.last_desc.read_framebuffer, source.fbo());
  EXPECT_EQ(state.last_desc.draw_framebuffer, destination.fbo());
  EXPECT_EQ(state.last_desc.read_color_attachment_index, 0u);
  EXPECT_EQ(state.last_desc.draw_color_attachment_index, 0u);
  EXPECT_EQ(state.last_desc.read_origin, glm::ivec2(0, 0));
  EXPECT_EQ(state.last_desc.draw_origin, glm::ivec2(0, 0));
  EXPECT_EQ(state.last_desc.size, glm::ivec2(8, 6));
  EXPECT_EQ(state.last_desc.draw_size, glm::ivec2(0, 0));
  EXPECT_EQ(state.last_desc.mask, cg::rhi::ClearMask::kColor);
  EXPECT_EQ(state.last_desc.filter, cg::rhi::FilterMode::kNearest);
}

TEST(renderer_state_test, FramebufferBlitPropagatesExplicitBlitOptions) {
  cg::test::ScopedFakeRendererDevice scoped_device;

  cg::Framebuffer source;
  cg::Framebuffer::Option source_option;
  source_option.size = {8, 6};
  source_option.attachments = {cg::kAttachmentColor};
  source.Init(source_option);

  cg::Framebuffer destination;
  cg::Framebuffer::Option destination_option;
  destination_option.size = {12, 10};
  destination_option.attachments = {cg::kAttachmentColor};
  destination.Init(destination_option);

  cg::Framebuffer::BlitOption blit_option;
  blit_option.read_color_attachment_index = 2;
  blit_option.draw_color_attachment_index = 1;
  blit_option.read_buffer = cg::rhi::ReadBuffer::kFront;
  blit_option.draw_buffer = cg::rhi::ReadBuffer::kBack;
  blit_option.read_origin = {1, 2};
  blit_option.draw_origin = {3, 4};
  blit_option.size = {5, 4};
  blit_option.draw_size = {7, 6};
  blit_option.mask = cg::rhi::ClearMask::kColor | cg::rhi::ClearMask::kDepth;
  blit_option.filter = cg::rhi::FilterMode::kLinear;

  source.Blit(&destination, blit_option);

  const cg::test::FakeBlitFramebufferState& state = scoped_device.device().blit_framebuffer_state();
  EXPECT_EQ(state.call_count, 1);
  EXPECT_EQ(state.last_desc.read_framebuffer, source.fbo());
  EXPECT_EQ(state.last_desc.draw_framebuffer, destination.fbo());
  EXPECT_EQ(state.last_desc.read_color_attachment_index, 2u);
  EXPECT_EQ(state.last_desc.draw_color_attachment_index, 1u);
  EXPECT_EQ(state.last_desc.read_buffer, cg::rhi::ReadBuffer::kFront);
  EXPECT_EQ(state.last_desc.draw_buffer, cg::rhi::ReadBuffer::kBack);
  EXPECT_EQ(state.last_desc.read_origin, glm::ivec2(1, 2));
  EXPECT_EQ(state.last_desc.draw_origin, glm::ivec2(3, 4));
  EXPECT_EQ(state.last_desc.size, glm::ivec2(5, 4));
  EXPECT_EQ(state.last_desc.draw_size, glm::ivec2(7, 6));
  EXPECT_EQ(state.last_desc.mask, cg::rhi::ClearMask::kColor | cg::rhi::ClearMask::kDepth);
  EXPECT_EQ(state.last_desc.filter, cg::rhi::FilterMode::kLinear);
}

TEST(renderer_state_test, ReadPixelsUsesDescriptorBasedReadbackContract) {
  cg::test::ScopedFakeRendererDevice scoped_device;

  cg::rhi::ReadPixelsDesc desc;
  desc.framebuffer = 9;
  desc.attachment_type = cg::rhi::AttachmentType::kDepth;
  desc.attachment_index = 1;
  desc.read_buffer = cg::rhi::ReadBuffer::kFront;
  desc.origin = {2, 3};
  desc.size = {4, 5};
  desc.format = cg::rhi::PixelFormat::kDepthComponent;
  desc.type = cg::rhi::PixelType::kFloat32;

  cg::rhi::GetDevice().ReadPixels(desc, nullptr);

  const cg::test::FakeReadPixelsState& state = scoped_device.device().read_pixels_state();
  EXPECT_EQ(state.call_count, 1);
  EXPECT_EQ(state.last_desc.framebuffer, 9u);
  EXPECT_EQ(state.last_desc.attachment_type, cg::rhi::AttachmentType::kDepth);
  EXPECT_EQ(state.last_desc.attachment_index, 1u);
  EXPECT_EQ(state.last_desc.read_buffer, cg::rhi::ReadBuffer::kFront);
  EXPECT_EQ(state.last_desc.origin, glm::ivec2(2, 3));
  EXPECT_EQ(state.last_desc.size, glm::ivec2(4, 5));
  EXPECT_EQ(state.last_desc.format, cg::rhi::PixelFormat::kDepthComponent);
  EXPECT_EQ(state.last_desc.type, cg::rhi::PixelType::kFloat32);
}

TEST(renderer_state_test, ShaderProgramAppliesBatchedBindingsThroughRhiProgram) {
  cg::test::ScopedFakeRendererDevice scoped_device;

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
  meta.min_filter = cg::rhi::FilterMode::kNearest;
  meta.mag_filter = cg::rhi::FilterMode::kNearest;
  meta.wrap_s = cg::rhi::WrapMode::kClampToEdge;
  meta.wrap_t = cg::rhi::WrapMode::kClampToEdge;

  cg::Texture texture(meta);
  const std::array<uint8_t, 4> pixel = {7u, 8u, 9u, 255u};
  texture.SetCpuLevelData(0, pixel.data(), pixel.size());

  cg::ShaderProgram program({
      .name = "fake",
      .kind = cg::rhi::ProgramKind::kRender,
  });

  cg::ShaderProgramBindings bindings;
  bindings.SetBool("enabled", true);
  bindings.SetInt("light_count", 3);
  bindings.SetFloat("exposure", 1.25f);
  bindings.SetVec3("camera.pos", {1.0f, 2.0f, 3.0f});
  bindings.SetTexture("albedo", texture);
  program.ApplyBindings(bindings);

  const cg::test::FakeProgramState& state = scoped_device.device().last_program_state();
  EXPECT_EQ(state.activation_call_count, 1);
  EXPECT_EQ(state.apply_bindings_call_count, 1);
  EXPECT_TRUE(state.bool_uniforms.at("enabled"));
  EXPECT_EQ(state.int_uniforms.at("light_count"), 3);
  EXPECT_FLOAT_EQ(state.float_uniforms.at("exposure"), 1.25f);
  ExpectVec3Near(state.vec3_uniforms.at("camera.pos"), glm::vec3(1.0f, 2.0f, 3.0f));
  EXPECT_EQ(state.texture_ids.at("albedo"), cg::test::TextureToken(texture));
}

TEST(renderer_state_test, ShaderProgramCreationUsesDescriptorBasedRhiContract) {
  cg::test::ScopedFakeRendererDevice scoped_device;

  const cg::ShaderProgram render_program({
      .name = "fake_render",
      .kind = cg::rhi::ProgramKind::kRender,
      .vs = {cg::ShaderCodePart{.glsl_path = "shader.vert", .code = "void main() {}"}},
      .fs = {cg::ShaderCodePart{.glsl_path = "shader.frag", .code = "void main() {}"}},
  });
  const cg::ShaderProgram compute_program({
      .name = "fake_compute",
      .kind = cg::rhi::ProgramKind::kCompute,
      .cs = {cg::ShaderCodePart{.glsl_path = "shader.comp", .code = "void main() {}"}},
  });
  (void)render_program;
  (void)compute_program;

  const auto& created_programs = scoped_device.device().created_programs();
  ASSERT_EQ(created_programs.size(), 2u);
  EXPECT_EQ(created_programs[0]->create_desc.name, "fake_render");
  EXPECT_EQ(created_programs[0]->create_desc.kind, cg::rhi::ProgramKind::kRender);
  ASSERT_EQ(created_programs[0]->create_desc.vs.size(), 1u);
  ASSERT_EQ(created_programs[0]->create_desc.fs.size(), 1u);
  EXPECT_EQ(created_programs[0]->create_desc.vs[0].glsl_path, "shader.vert");
  EXPECT_EQ(created_programs[0]->create_desc.fs[0].glsl_path, "shader.frag");
  EXPECT_TRUE(created_programs[0]->create_desc.cs.empty());

  EXPECT_EQ(created_programs[1]->create_desc.name, "fake_compute");
  EXPECT_EQ(created_programs[1]->create_desc.kind, cg::rhi::ProgramKind::kCompute);
  ASSERT_EQ(created_programs[1]->create_desc.cs.size(), 1u);
  EXPECT_EQ(created_programs[1]->create_desc.cs[0].glsl_path, "shader.comp");
  EXPECT_TRUE(created_programs[1]->create_desc.vs.empty());
  EXPECT_TRUE(created_programs[1]->create_desc.fs.empty());
}

TEST(renderer_state_test, ShaderProgramApplyBindingsBatchesMergedBindingsThroughRhiProgram) {
  cg::test::ScopedFakeRendererDevice scoped_device;

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
  meta.min_filter = cg::rhi::FilterMode::kNearest;
  meta.mag_filter = cg::rhi::FilterMode::kNearest;
  meta.wrap_s = cg::rhi::WrapMode::kClampToEdge;
  meta.wrap_t = cg::rhi::WrapMode::kClampToEdge;

  cg::Texture texture(meta);
  const std::array<uint8_t, 4> pixel = {17u, 18u, 19u, 255u};
  texture.SetCpuLevelData(0, pixel.data(), pixel.size());

  std::unique_ptr<cg::rhi::Buffer> buffer = cg::rhi::GetDevice().CreateBuffer(cg::rhi::BufferType::kStorage);
  ASSERT_TRUE(buffer != nullptr);

  cg::ShaderProgramBindings common_bindings;
  common_bindings.SetBool("enabled", true);
  common_bindings.SetTexture("albedo", texture);

  cg::ShaderProgramBindings extra_bindings;
  extra_bindings.SetInt("light_count", 5);
  extra_bindings.SetBufferBinding({
      .buffer = buffer.get(),
      .binding_point = 3,
  });
  common_bindings.Append(extra_bindings);

  cg::ShaderProgram program({
      .name = "fake",
      .kind = cg::rhi::ProgramKind::kRender,
  });
  program.ApplyBindings(common_bindings);

  const cg::test::FakeProgramState& program_state = scoped_device.device().last_program_state();
  EXPECT_EQ(program_state.activation_call_count, 1);
  EXPECT_EQ(program_state.apply_bindings_call_count, 1);
  EXPECT_TRUE(program_state.bool_uniforms.at("enabled"));
  EXPECT_EQ(program_state.int_uniforms.at("light_count"), 5);
  EXPECT_EQ(program_state.texture_ids.at("albedo"), cg::test::TextureToken(texture));

  const cg::test::FakeBufferBindingState& buffer_state = scoped_device.device().buffer_binding_state();
  EXPECT_EQ(buffer_state.apply_call_count, 1);
  ASSERT_EQ(buffer_state.calls.size(), 1u);
  EXPECT_EQ(buffer_state.calls[0].buffer_type, cg::rhi::BufferType::kStorage);
  EXPECT_EQ(buffer_state.calls[0].binding_point, 3u);
}

TEST(renderer_state_test, ShaderProgramDrawBindingsSubmitsBindingsAndDrawTogether) {
  cg::test::ScopedFakeRendererDevice scoped_device;

  std::unique_ptr<cg::rhi::VertexArray> vertex_array = cg::rhi::GetDevice().CreateVertexArray();
  ASSERT_TRUE(vertex_array != nullptr);

  cg::ShaderProgram program({
      .name = "fake_draw",
      .kind = cg::rhi::ProgramKind::kRender,
  });

  cg::ShaderProgramBindings bindings;
  bindings.SetInt("material_index", 4);

  program.DrawBindings(bindings, {
      .kind = cg::rhi::DrawKind::kArrays,
      .topology = cg::rhi::PrimitiveTopology::kTriangles,
      .vertex_array = vertex_array.get(),
      .first = 2,
      .count = 6,
      .instance_count = 3,
  });

  const cg::test::FakeProgramState& program_state = scoped_device.device().last_program_state();
  EXPECT_EQ(program_state.activation_call_count, 1);
  EXPECT_EQ(program_state.apply_bindings_call_count, 1);
  EXPECT_EQ(program_state.int_uniforms.at("material_index"), 4);

  const auto& draw_calls = scoped_device.device().draw_calls();
  ASSERT_EQ(draw_calls.size(), 1u);
  EXPECT_EQ(draw_calls.front().kind, cg::rhi::DrawKind::kArrays);
  EXPECT_EQ(draw_calls.front().topology, cg::rhi::PrimitiveTopology::kTriangles);
  EXPECT_TRUE(draw_calls.front().has_vertex_array);
  EXPECT_FALSE(draw_calls.front().has_index_buffer);
  EXPECT_EQ(draw_calls.front().first, 2u);
  EXPECT_EQ(draw_calls.front().count, 6u);
  EXPECT_EQ(draw_calls.front().instance_count, 3u);
}

TEST(renderer_state_test, ShaderBindingHelpersAppendReusableSceneStateIntoSingleSubmission) {
  cg::test::ScopedFakeRendererDevice scoped_device;

  cg::Camera camera;
  camera.SetAspect(16.0f / 9.0f);
  camera.SetPerspectiveFov(45.0f);
  camera.SetNearClip(0.25f);
  camera.SetFarClip(80.0f);
  camera.mutable_transform()->SetTranslation({1.0f, 2.0f, 3.0f});
  camera.mutable_transform()->SetRotation(glm::normalize(glm::quat(1.0f, 0.2f, -0.1f, 0.05f)));

  cg::Transform model_transform;
  model_transform.SetTranslation({4.0f, 5.0f, 6.0f});
  model_transform.SetScale({2.0f, 3.0f, 4.0f});
  const glm::mat4 model = model_transform.GetModelMatrix();

  cg::ShaderProgramBindings bindings;
  cg::AppendModelBindings(model, &bindings);
  cg::AppendCameraBindings(camera, &bindings);
  cg::AppendCameraBindings(camera, "camera_1", &bindings);
  cg::AppendMaterialIndexBindings(7, &bindings);
  cg::AppendPrimitiveStartIndexBindings(11, &bindings);
  cg::AppendResolutionBindings({640.0f, 360.0f}, &bindings);
  cg::AppendFrameNumBindings(13, &bindings);
  cg::AppendDirtyBindings(true, &bindings);

  cg::ShaderProgram program({
      .name = "fake",
      .kind = cg::rhi::ProgramKind::kRender,
  });
  program.ApplyBindings(bindings);

  const cg::test::FakeProgramState& state = scoped_device.device().last_program_state();
  EXPECT_EQ(state.activation_call_count, 1);
  EXPECT_EQ(state.apply_bindings_call_count, 1);
  ExpectMat4Near(state.mat4_uniforms.at("model"), model);
  ExpectVec3Near(state.vec3_uniforms.at("camera.pos_ws"), camera.transform().translation());
  ExpectVec3Near(state.vec3_uniforms.at("camera.front"), camera.front_ws());
  ExpectMat4Near(state.mat4_uniforms.at("camera.view"), camera.GetViewMatrix());
  ExpectMat4Near(state.mat4_uniforms.at("camera.project"), camera.GetProjectMatrix());
  EXPECT_FLOAT_EQ(state.float_uniforms.at("camera.near"), 0.25f);
  EXPECT_FLOAT_EQ(state.float_uniforms.at("camera.far"), 80.0f);
  ExpectVec3Near(state.vec3_uniforms.at("camera_1.pos_ws"), camera.transform().translation());
  ExpectVec3Near(state.vec3_uniforms.at("camera_1.front"), camera.front_ws());
  EXPECT_EQ(state.int_uniforms.at("material_index"), 7);
  EXPECT_EQ(state.int_uniforms.at("primitive_start_index"), 11);
  EXPECT_EQ(state.int_uniforms.at("frame_num"), 13);
  ExpectVec2Near(state.vec2_uniforms.at("resolution"), {640.0f, 360.0f});
  EXPECT_TRUE(state.bool_uniforms.at("dirty"));
}

TEST(renderer_state_test, RenderObjectBindingHelpersAppendModelAndCameraIntoSingleSubmission) {
  cg::test::ScopedFakeRendererDevice scoped_device;

  cg::Camera camera;
  camera.SetAspect(4.0f / 3.0f);
  camera.SetPerspectiveFov(50.0f);
  camera.SetNearClip(0.5f);
  camera.SetFarClip(120.0f);
  camera.mutable_transform()->SetTranslation({-2.0f, 1.5f, 7.0f});
  camera.mutable_transform()->SetRotation(glm::normalize(glm::quat(1.0f, -0.15f, 0.25f, 0.05f)));

  cg::Object object;
  object.transform.SetTranslation({3.0f, 4.0f, 5.0f});
  object.transform.SetRotation(glm::normalize(glm::quat(1.0f, 0.1f, 0.2f, -0.05f)));
  object.transform.SetScale({1.5f, 2.0f, 2.5f});

  cg::ShaderProgramBindings bindings;
  cg::AppendRenderObjectBindings(object, camera, &bindings);
  cg::AppendMaterialIndexBindings(9, &bindings);

  cg::ShaderProgram program({
      .name = "fake",
      .kind = cg::rhi::ProgramKind::kRender,
  });
  program.ApplyBindings(bindings);

  const cg::test::FakeProgramState& state = scoped_device.device().last_program_state();
  EXPECT_EQ(state.activation_call_count, 1);
  EXPECT_EQ(state.apply_bindings_call_count, 1);
  ExpectMat4Near(state.mat4_uniforms.at("model"), object.transform.GetModelMatrix());
  ExpectVec3Near(state.vec3_uniforms.at("camera.pos_ws"), camera.transform().translation());
  ExpectVec3Near(state.vec3_uniforms.at("camera.front"), camera.front_ws());
  ExpectMat4Near(state.mat4_uniforms.at("camera.view"), camera.GetViewMatrix());
  ExpectMat4Near(state.mat4_uniforms.at("camera.project"), camera.GetProjectMatrix());
  EXPECT_FLOAT_EQ(state.float_uniforms.at("camera.near"), 0.5f);
  EXPECT_FLOAT_EQ(state.float_uniforms.at("camera.far"), 120.0f);
  EXPECT_EQ(state.int_uniforms.at("material_index"), 9);
}

TEST(renderer_state_test, LegacyRenderBindingHelpersAppendViewProjectAndViewPosIntoSingleSubmission) {
  cg::test::ScopedFakeRendererDevice scoped_device;

  cg::Camera camera;
  camera.SetAspect(16.0f / 10.0f);
  camera.SetPerspectiveFov(55.0f);
  camera.mutable_transform()->SetTranslation({6.0f, 5.0f, 4.0f});

  cg::Object object;
  object.transform.SetTranslation({1.0f, 2.0f, 3.0f});
  object.transform.SetScale({1.5f, 2.0f, 2.5f});

  cg::ShaderProgramBindings bindings;
  cg::AppendLegacyRenderObjectBindings(object, camera, &bindings);
  cg::AppendViewPosBindings(camera, &bindings);

  cg::ShaderProgram program({
      .name = "fake_legacy",
      .kind = cg::rhi::ProgramKind::kRender,
  });
  program.ApplyBindings(bindings);

  const cg::test::FakeProgramState& state = scoped_device.device().last_program_state();
  EXPECT_EQ(state.activation_call_count, 1);
  EXPECT_EQ(state.apply_bindings_call_count, 1);
  ExpectMat4Near(state.mat4_uniforms.at("model"), object.transform.GetModelMatrix());
  ExpectMat4Near(state.mat4_uniforms.at("view"), camera.GetViewMatrix());
  ExpectMat4Near(state.mat4_uniforms.at("project"), camera.GetProjectMatrix());
  ExpectVec3Near(state.vec3_uniforms.at("view_pos_ws"), camera.transform().translation());
}

TEST(renderer_state_test, RenderShaderMergesSceneCommonBindingsIntoSingleDrawSubmission) {
  cg::test::ScopedFakeRendererDevice scoped_device;
  EnteredTestScene entered_scene;
  entered_scene.scene().AddObject({
      .object_name = "cube",
      .transform = cg::Transform(),
      .mesh_or_model_name = "cube",
      .material_name = "gold",
  });

  TestCommonRenderShader shader(entered_scene.scene(), entered_scene.scene().GetObject("cube"));

  const cg::test::FakeProgramState& program_state = scoped_device.device().last_program_state();
  EXPECT_EQ(program_state.activation_call_count, 1);
  EXPECT_EQ(program_state.apply_bindings_call_count, 1);
  EXPECT_EQ(program_state.int_uniforms.at("light_repo_num"), 0);
  EXPECT_EQ(program_state.int_uniforms.at("material_repo_num"),
            entered_scene.scene().material_repo().num());
  ExpectMat4Near(program_state.mat4_uniforms.at("model"),
                 entered_scene.scene().GetObject("cube").transform.GetModelMatrix());
  ExpectMat4Near(program_state.mat4_uniforms.at("camera.view"),
                 entered_scene.scene().camera().GetViewMatrix());

  const cg::test::FakeBufferBindingState& buffer_state = scoped_device.device().buffer_binding_state();
  EXPECT_EQ(buffer_state.apply_call_count, 1);

  const auto& draw_calls = scoped_device.device().draw_calls();
  ASSERT_EQ(draw_calls.size(), 1u);
  EXPECT_TRUE(draw_calls.front().has_vertex_array);
}

TEST(renderer_state_test, ComputeShaderMergesSceneCommonBindingsIntoSingleDispatchSubmission) {
  cg::test::ScopedFakeRendererDevice scoped_device;
  EnteredTestScene entered_scene;

  TestCommonComputeShader shader(entered_scene.scene());

  const cg::test::FakeProgramState& program_state = scoped_device.device().last_program_state();
  EXPECT_EQ(program_state.activation_call_count, 1);
  EXPECT_EQ(program_state.apply_bindings_call_count, 1);
  EXPECT_EQ(program_state.int_uniforms.at("light_repo_num"), 0);
  EXPECT_EQ(program_state.int_uniforms.at("material_repo_num"),
            entered_scene.scene().material_repo().num());
  ExpectVec2Near(program_state.vec2_uniforms.at("resolution"), {64.0f, 32.0f});

  const cg::test::FakeBufferBindingState& buffer_state = scoped_device.device().buffer_binding_state();
  EXPECT_EQ(buffer_state.apply_call_count, 1);

  const cg::test::FakeDispatchComputeState& dispatch_state = scoped_device.device().dispatch_compute_state();
  EXPECT_EQ(dispatch_state.call_count, 1);
  EXPECT_EQ(dispatch_state.last_desc.workgroup_count, glm::uvec3(2, 3, 1));
}

TEST(renderer_state_test, TextureShaderUsesLegacyTransformBindingsAndSingleDrawSubmission) {
  cg::test::ScopedFakeRendererDevice scoped_device;
  EnteredTestScene entered_scene;
  entered_scene.scene().AddObject({
      .object_name = "quad",
      .transform = cg::Transform(),
      .mesh_or_model_name = "plane",
      .material_name = "gold",
  });

  const cg::Texture texture = MakeSolidTexture({21u, 22u, 23u, 255u});
  cg::TextureShader({texture}, entered_scene.scene(), entered_scene.scene().GetObject("quad"));

  const cg::test::FakeProgramState& state = scoped_device.device().last_program_state();
  EXPECT_EQ(state.activation_call_count, 1);
  EXPECT_EQ(state.apply_bindings_call_count, 1);
  ExpectMat4Near(state.mat4_uniforms.at("model"),
                 entered_scene.scene().GetObject("quad").transform.GetModelMatrix());
  ExpectMat4Near(state.mat4_uniforms.at("view"), entered_scene.scene().camera().GetViewMatrix());
  ExpectMat4Near(state.mat4_uniforms.at("project"), entered_scene.scene().camera().GetProjectMatrix());
  EXPECT_EQ(state.texture_ids.count("texture0"), 1u);

  const auto& draw_calls = scoped_device.device().draw_calls();
  ASSERT_EQ(draw_calls.size(), 1u);
  EXPECT_TRUE(draw_calls.front().has_vertex_array);
}

TEST(renderer_state_test, Texture2DLodShaderForwardsViewPosWorldInSingleDrawSubmission) {
  cg::test::ScopedFakeRendererDevice scoped_device;
  EnteredTestScene entered_scene;
  entered_scene.scene().AddObject({
      .object_name = "cube",
      .transform = cg::Transform(),
      .mesh_or_model_name = "cube",
      .material_name = "gold",
  });

  const cg::Texture texture = MakeSolidTexture({31u, 32u, 33u, 255u});
  const glm::vec3 view_pos_ws(9.0f, 8.0f, 7.0f);
  cg::Texture2DLodShader({texture, view_pos_ws}, entered_scene.scene(), entered_scene.scene().GetObject("cube"));

  const cg::test::FakeProgramState& state = scoped_device.device().last_program_state();
  EXPECT_EQ(state.activation_call_count, 1);
  EXPECT_EQ(state.apply_bindings_call_count, 1);
  EXPECT_EQ(state.texture_ids.count("texture2D0"), 1u);
  ExpectVec3Near(state.vec3_uniforms.at("view_pos_ws"), view_pos_ws);

  const auto& draw_calls = scoped_device.device().draw_calls();
  ASSERT_EQ(draw_calls.size(), 1u);
  EXPECT_TRUE(draw_calls.front().has_vertex_array);
}

TEST(renderer_state_test, ShaderProgramAppliesStorageTextureBindingsThroughProgramBindings) {
  cg::test::ScopedFakeRendererDevice scoped_device;

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
  meta.min_filter = cg::rhi::FilterMode::kNearest;
  meta.mag_filter = cg::rhi::FilterMode::kNearest;
  meta.wrap_s = cg::rhi::WrapMode::kClampToEdge;
  meta.wrap_t = cg::rhi::WrapMode::kClampToEdge;

  cg::Texture sampled(meta);
  sampled.SetCpuLevelData(0, std::array<uint8_t, 4>{1u, 2u, 3u, 255u}.data(), 4);
  cg::Texture storage(meta);
  storage.SetCpuLevelData(0, std::array<uint8_t, 4>{4u, 5u, 6u, 255u}.data(), 4);

  cg::ShaderProgram program({
      .name = "fake",
      .kind = cg::rhi::ProgramKind::kRender,
  });

  cg::ShaderProgramBindings bindings;
  bindings.SetTexture("albedo", sampled);
  bindings.SetStorageTexture("canvas", storage, cg::rhi::TextureAccess::kWriteOnly);
  program.ApplyBindings(bindings);

  const cg::test::FakeProgramState& state = scoped_device.device().last_program_state();
  EXPECT_EQ(state.activation_call_count, 1);
  EXPECT_EQ(state.texture_ids.at("albedo"), cg::test::TextureToken(sampled));
  EXPECT_EQ(state.texture_ids.at("canvas"), cg::test::TextureToken(storage));

  const auto& storage_calls = scoped_device.device().storage_texture_binding_calls();
  ASSERT_EQ(storage_calls.size(), 1u);
  EXPECT_EQ(storage_calls[0].texture_unit, 1u);
  EXPECT_EQ(storage_calls[0].texture_id, cg::test::TextureToken(storage));
  EXPECT_EQ(storage_calls[0].access, cg::rhi::TextureAccess::kWriteOnly);
}

TEST(renderer_state_test, ShaderProgramDispatchesComputeThroughRhiDevice) {
  cg::test::ScopedFakeRendererDevice scoped_device;

  cg::ShaderProgram program({
      .name = "fake_compute",
      .kind = cg::rhi::ProgramKind::kCompute,
  });
  program.DispatchCompute({
      .workgroup_count = glm::uvec3(4, 2, 1),
  });

  const cg::test::FakeProgramState& state = scoped_device.device().last_program_state();
  EXPECT_EQ(state.activation_call_count, 1);

  const cg::test::FakeDispatchComputeState& dispatch_state = scoped_device.device().dispatch_compute_state();
  EXPECT_EQ(dispatch_state.call_count, 1);
  EXPECT_EQ(dispatch_state.last_desc.workgroup_count, glm::uvec3(4, 2, 1));
  EXPECT_EQ(dispatch_state.last_desc.barrier, cg::rhi::MemoryBarrier::kAll);
}

TEST(renderer_state_test, ShaderProgramDispatchComputeBindingsSubmitsBindingsAndDispatchTogether) {
  cg::test::ScopedFakeRendererDevice scoped_device;

  cg::ShaderProgram program({
      .name = "fake_compute",
      .kind = cg::rhi::ProgramKind::kCompute,
  });
  cg::ShaderProgramBindings bindings;
  bindings.SetInt("frame_num", 8);

  program.DispatchComputeBindings(bindings, {
      .workgroup_count = glm::uvec3(2, 1, 1),
      .barrier = cg::rhi::MemoryBarrier::kAll,
  });

  const cg::test::FakeProgramState& state = scoped_device.device().last_program_state();
  EXPECT_EQ(state.activation_call_count, 1);
  EXPECT_EQ(state.apply_bindings_call_count, 1);
  EXPECT_EQ(state.int_uniforms.at("frame_num"), 8);

  const cg::test::FakeDispatchComputeState& dispatch_state = scoped_device.device().dispatch_compute_state();
  EXPECT_EQ(dispatch_state.call_count, 1);
  EXPECT_EQ(dispatch_state.last_desc.workgroup_count, glm::uvec3(2, 1, 1));
  EXPECT_EQ(dispatch_state.last_desc.barrier, cg::rhi::MemoryBarrier::kAll);
}

TEST(renderer_state_test, ShaderProgramAppliesBufferBindingsThroughRhiDevice) {
  cg::test::ScopedFakeRendererDevice scoped_device;

  std::unique_ptr<cg::rhi::Buffer> buffer = cg::rhi::GetDevice().CreateBuffer(cg::rhi::BufferType::kStorage);
  ASSERT_TRUE(buffer != nullptr);

  cg::ShaderProgram program({
      .name = "fake",
      .kind = cg::rhi::ProgramKind::kRender,
  });

  cg::ShaderProgramBindings bindings;
  bindings.SetBufferBinding({
      .buffer = buffer.get(),
      .binding_point = 9,
  });
  program.ApplyBindings(bindings);

  const cg::test::FakeProgramState& program_state = scoped_device.device().last_program_state();
  EXPECT_EQ(program_state.activation_call_count, 1);
  EXPECT_EQ(program_state.apply_bindings_call_count, 1);

  const cg::test::FakeBufferBindingState& buffer_state = scoped_device.device().buffer_binding_state();
  EXPECT_EQ(buffer_state.apply_call_count, 1);
  ASSERT_EQ(buffer_state.calls.size(), 1u);
  EXPECT_EQ(buffer_state.calls[0].buffer_type, cg::rhi::BufferType::kStorage);
  EXPECT_EQ(buffer_state.calls[0].binding_point, 9u);
}

TEST(renderer_state_test, RenderStateDescriptorAppliesOnlySpecifiedState) {
  cg::test::ScopedFakeRendererDevice scoped_device;

  cg::rhi::GetDevice().ApplyRenderState({
      .depth_test_enabled = true,
      .cull_enabled = true,
      .cull_mode = cg::rhi::CullMode::kFront,
      .front_face = cg::rhi::FrontFace::kClockwise,
  });

  const cg::test::FakeRenderState& first_state = scoped_device.device().render_state();
  EXPECT_EQ(first_state.apply_call_count, 1);
  EXPECT_TRUE(first_state.depth_test_enabled);
  EXPECT_TRUE(first_state.cull_enabled);
  EXPECT_EQ(first_state.cull_mode, cg::rhi::CullMode::kFront);
  EXPECT_EQ(first_state.front_face, cg::rhi::FrontFace::kClockwise);

  cg::rhi::GetDevice().ApplyRenderState({
      .depth_test_enabled = false,
      .cull_mode = cg::rhi::CullMode::kBack,
  });

  const cg::test::FakeRenderState& second_state = scoped_device.device().render_state();
  EXPECT_EQ(second_state.apply_call_count, 2);
  EXPECT_FALSE(second_state.depth_test_enabled);
  EXPECT_TRUE(second_state.cull_enabled);
  EXPECT_EQ(second_state.cull_mode, cg::rhi::CullMode::kBack);
  EXPECT_EQ(second_state.front_face, cg::rhi::FrontFace::kClockwise);
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
