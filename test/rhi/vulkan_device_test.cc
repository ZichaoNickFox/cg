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

cg::Texture MakeRgbaTexture(int width, int height, const std::array<uint8_t, 4>& rgba) {
  cg::Texture::Meta meta;
  meta.type = cg::Texture::kTexture2D;
  meta.width = width;
  meta.height = height;
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

  std::vector<uint8_t> pixels(static_cast<size_t>(width * height * 4));
  for (size_t offset = 0; offset + rgba.size() <= pixels.size(); offset += rgba.size()) {
    std::copy(rgba.begin(), rgba.end(), pixels.begin() + static_cast<std::ptrdiff_t>(offset));
  }

  cg::Texture texture(meta);
  texture.SetCpuLevelData(0, pixels.data(), pixels.size());
  return texture;
}

std::array<uint8_t, 4> PatternPixel(int row, int column) {
  return {
      static_cast<uint8_t>(row * 40 + column * 3 + 1),
      static_cast<uint8_t>(row * 40 + column * 3 + 2),
      static_cast<uint8_t>(row * 40 + column * 3 + 3),
      255u,
  };
}

float PatternDepthValue(int row, int column) {
  return static_cast<float>(row * 4 + column + 1) / 16.0f;
}

uint8_t PatternStencilValue(int row, int column) {
  return static_cast<uint8_t>(row * 16 + column + 1);
}

cg::Texture MakePatternRgbaTexture(int width, int height) {
  cg::Texture::Meta meta;
  meta.type = cg::Texture::kTexture2D;
  meta.width = width;
  meta.height = height;
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

  std::vector<uint8_t> pixels(static_cast<size_t>(width * height * 4));
  for (int row = 0; row < height; ++row) {
    for (int column = 0; column < width; ++column) {
      const std::array<uint8_t, 4> pixel = PatternPixel(row, column);
      const size_t offset = static_cast<size_t>(row * width + column) * 4;
      std::copy(pixel.begin(), pixel.end(), pixels.begin() + static_cast<std::ptrdiff_t>(offset));
    }
  }

  cg::Texture texture(meta);
  texture.SetCpuLevelData(0, pixels.data(), pixels.size());
  return texture;
}

cg::Texture MakeCornerRgbaTexture() {
  cg::Texture::Meta meta;
  meta.type = cg::Texture::kTexture2D;
  meta.width = 2;
  meta.height = 2;
  meta.channel_num = 4;
  meta.hdr = false;
  meta.level_num = 1;
  meta.depth = 1;
  meta.format = cg::rhi::TextureFormat::kRGBA8;
  meta.pixel_format = cg::rhi::PixelFormat::kRGBA;
  meta.pixel_type = cg::rhi::PixelType::kUInt8;
  meta.min_filter = cg::rhi::FilterMode::kLinear;
  meta.mag_filter = cg::rhi::FilterMode::kLinear;
  meta.wrap_s = cg::rhi::WrapMode::kClampToEdge;
  meta.wrap_t = cg::rhi::WrapMode::kClampToEdge;

  const std::array<uint8_t, 16> pixels = {
      0u,   0u,   0u,   255u,
      255u, 0u,   0u,   255u,
      0u,   255u, 0u,   255u,
      255u, 255u, 0u,   255u,
  };

  cg::Texture texture(meta);
  texture.SetCpuLevelData(0, pixels.data(), pixels.size());
  return texture;
}

cg::Texture MakeDepthTexture(int width, int height, float depth_value) {
  cg::Texture::Meta meta;
  meta.type = cg::Texture::kTexture2D;
  meta.width = width;
  meta.height = height;
  meta.channel_num = 1;
  meta.hdr = true;
  meta.level_num = 1;
  meta.depth = 1;
  meta.format = cg::rhi::TextureFormat::kDepth32F;
  meta.pixel_format = cg::rhi::PixelFormat::kDepthComponent;
  meta.pixel_type = cg::rhi::PixelType::kFloat32;
  meta.min_filter = cg::rhi::FilterMode::kNearest;
  meta.mag_filter = cg::rhi::FilterMode::kNearest;
  meta.wrap_s = cg::rhi::WrapMode::kClampToBorder;
  meta.wrap_t = cg::rhi::WrapMode::kClampToBorder;

  std::vector<float> pixels(static_cast<size_t>(width * height), depth_value);
  cg::Texture texture(meta);
  texture.SetCpuLevelData(0, pixels.data(), pixels.size() * sizeof(float));
  return texture;
}

cg::Texture MakePatternDepthTexture(int width, int height) {
  cg::Texture::Meta meta;
  meta.type = cg::Texture::kTexture2D;
  meta.width = width;
  meta.height = height;
  meta.channel_num = 1;
  meta.hdr = true;
  meta.level_num = 1;
  meta.depth = 1;
  meta.format = cg::rhi::TextureFormat::kDepth32F;
  meta.pixel_format = cg::rhi::PixelFormat::kDepthComponent;
  meta.pixel_type = cg::rhi::PixelType::kFloat32;
  meta.min_filter = cg::rhi::FilterMode::kNearest;
  meta.mag_filter = cg::rhi::FilterMode::kNearest;
  meta.wrap_s = cg::rhi::WrapMode::kClampToBorder;
  meta.wrap_t = cg::rhi::WrapMode::kClampToBorder;

  std::vector<float> pixels(static_cast<size_t>(width * height));
  for (int row = 0; row < height; ++row) {
    for (int column = 0; column < width; ++column) {
      pixels[static_cast<size_t>(row * width + column)] = PatternDepthValue(row, column);
    }
  }

  cg::Texture texture(meta);
  texture.SetCpuLevelData(0, pixels.data(), pixels.size() * sizeof(float));
  return texture;
}

cg::Texture MakeRgbaFloatTexture(int width, int height, const std::array<float, 4>& rgba) {
  cg::Texture::Meta meta;
  meta.type = cg::Texture::kTexture2D;
  meta.width = width;
  meta.height = height;
  meta.channel_num = 4;
  meta.hdr = true;
  meta.level_num = 1;
  meta.depth = 1;
  meta.format = cg::rhi::TextureFormat::kRGBA32F;
  meta.pixel_format = cg::rhi::PixelFormat::kRGBA;
  meta.pixel_type = cg::rhi::PixelType::kFloat32;
  meta.min_filter = cg::rhi::FilterMode::kNearest;
  meta.mag_filter = cg::rhi::FilterMode::kNearest;
  meta.wrap_s = cg::rhi::WrapMode::kClampToBorder;
  meta.wrap_t = cg::rhi::WrapMode::kClampToBorder;

  std::vector<float> pixels(static_cast<size_t>(width * height * 4));
  for (size_t offset = 0; offset + rgba.size() <= pixels.size(); offset += rgba.size()) {
    std::copy(rgba.begin(), rgba.end(), pixels.begin() + static_cast<std::ptrdiff_t>(offset));
  }

  cg::Texture texture(meta);
  texture.SetCpuLevelData(0, pixels.data(), pixels.size() * sizeof(float));
  return texture;
}

cg::Texture MakeCornerRgbaFloatTexture() {
  cg::Texture::Meta meta;
  meta.type = cg::Texture::kTexture2D;
  meta.width = 2;
  meta.height = 2;
  meta.channel_num = 4;
  meta.hdr = true;
  meta.level_num = 1;
  meta.depth = 1;
  meta.format = cg::rhi::TextureFormat::kRGBA32F;
  meta.pixel_format = cg::rhi::PixelFormat::kRGBA;
  meta.pixel_type = cg::rhi::PixelType::kFloat32;
  meta.min_filter = cg::rhi::FilterMode::kLinear;
  meta.mag_filter = cg::rhi::FilterMode::kLinear;
  meta.wrap_s = cg::rhi::WrapMode::kClampToEdge;
  meta.wrap_t = cg::rhi::WrapMode::kClampToEdge;

  const std::array<float, 16> pixels = {
      0.0f, 0.0f, 0.0f, 1.0f,
      1.0f, 0.0f, 0.0f, 1.0f,
      0.0f, 1.0f, 0.0f, 1.0f,
      1.0f, 1.0f, 0.0f, 1.0f,
  };

  cg::Texture texture(meta);
  texture.SetCpuLevelData(0, pixels.data(), pixels.size() * sizeof(float));
  return texture;
}

cg::Texture MakeStencilTexture(int width, int height, uint8_t stencil_value) {
  cg::Texture::Meta meta;
  meta.type = cg::Texture::kTexture2D;
  meta.width = width;
  meta.height = height;
  meta.channel_num = 1;
  meta.hdr = false;
  meta.level_num = 1;
  meta.depth = 1;
  meta.format = cg::rhi::TextureFormat::kStencil8;
  meta.pixel_format = cg::rhi::PixelFormat::kStencilIndex;
  meta.pixel_type = cg::rhi::PixelType::kUInt8;
  meta.min_filter = cg::rhi::FilterMode::kNearest;
  meta.mag_filter = cg::rhi::FilterMode::kNearest;
  meta.wrap_s = cg::rhi::WrapMode::kClampToBorder;
  meta.wrap_t = cg::rhi::WrapMode::kClampToBorder;

  std::vector<uint8_t> pixels(static_cast<size_t>(width * height), stencil_value);
  cg::Texture texture(meta);
  texture.SetCpuLevelData(0, pixels.data(), pixels.size());
  return texture;
}

cg::Texture MakePatternStencilTexture(int width, int height) {
  cg::Texture::Meta meta;
  meta.type = cg::Texture::kTexture2D;
  meta.width = width;
  meta.height = height;
  meta.channel_num = 1;
  meta.hdr = false;
  meta.level_num = 1;
  meta.depth = 1;
  meta.format = cg::rhi::TextureFormat::kStencil8;
  meta.pixel_format = cg::rhi::PixelFormat::kStencilIndex;
  meta.pixel_type = cg::rhi::PixelType::kUInt8;
  meta.min_filter = cg::rhi::FilterMode::kNearest;
  meta.mag_filter = cg::rhi::FilterMode::kNearest;
  meta.wrap_s = cg::rhi::WrapMode::kClampToBorder;
  meta.wrap_t = cg::rhi::WrapMode::kClampToBorder;

  std::vector<uint8_t> pixels(static_cast<size_t>(width * height));
  for (int row = 0; row < height; ++row) {
    for (int column = 0; column < width; ++column) {
      pixels[static_cast<size_t>(row * width + column)] = PatternStencilValue(row, column);
    }
  }

  cg::Texture texture(meta);
  texture.SetCpuLevelData(0, pixels.data(), pixels.size());
  return texture;
}

class ProbeBuffer final : public cg::rhi::Buffer {
 public:
  explicit ProbeBuffer(cg::rhi::BufferType type) : type_(type) {}

  cg::rhi::BufferType type() const override { return type_; }
  void SetData(size_t, const void*, cg::rhi::BufferUsage) override {}
  void UpdateData(size_t, size_t, const void*) override {}
  void BindBase(uint32_t binding_point) override { bound_base_points_.push_back(binding_point); }
  void ReadData(const cg::rhi::BufferReadDesc&, void*) override {}

  const std::vector<uint32_t>& bound_base_points() const { return bound_base_points_; }

 private:
  cg::rhi::BufferType type_ = cg::rhi::BufferType::kVertex;
  std::vector<uint32_t> bound_base_points_;
};

TEST(vulkan_device_test, StandaloneDeviceSupportsSyntheticTextureLifecycle) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  EXPECT_EQ(device->capabilities().backend, cg::rhi::BackendType::kVulkan);
  EXPECT_EQ(device->capabilities().scene_backend, cg::rhi::BackendType::kVulkan);
  EXPECT_EQ(device->capabilities().scene_api_name, "Vulkan Synthetic RHI");
  EXPECT_EQ(device->capabilities().shader_language_name, "SPIR-V (synthetic)");
  EXPECT_TRUE(device->capabilities().supports_glsl_450);
  EXPECT_TRUE(device->capabilities().supports_storage_buffers);
  EXPECT_TRUE(device->capabilities().supports_storage_images);

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
      cg::rhi::GetDevice().CreateProgram({
          .name = "synthetic",
          .kind = cg::rhi::ProgramKind::kRender,
      });
  ASSERT_TRUE(program != nullptr);
  cg::rhi::ProgramBindings bindings;
  bindings.textures.push_back({"albedo", &texture});
  cg::Texture other_texture = MakeRgbaTexture(1, 1, {9u, 8u, 7u, 6u});
  bindings.textures.push_back({"normal", &other_texture});
  program->ApplyBindings(bindings);

  texture.ReleaseGpuResources();
  EXPECT_FALSE(texture.storage()->uploaded_to_gl);
  EXPECT_EQ(texture.storage()->id, std::numeric_limits<uint32_t>::max());
}

TEST(vulkan_device_test, StandaloneDeviceAcceptsRepeatedBatchBindingsWithTextures) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::Texture albedo = MakeRgbaTexture(1, 1, {1u, 2u, 3u, 4u});
  cg::Texture normal = MakeRgbaTexture(1, 1, {5u, 6u, 7u, 8u});

  const std::shared_ptr<cg::rhi::Program> program =
      cg::rhi::GetDevice().CreateProgram({
          .name = "synthetic_batch",
          .kind = cg::rhi::ProgramKind::kRender,
      });
  ASSERT_TRUE(program != nullptr);

  cg::rhi::ProgramBindings bindings;
  bindings.uniforms.push_back({"enabled", true});
  bindings.uniforms.push_back({"frame_num", 12});
  bindings.textures.push_back({"albedo", &albedo});
  bindings.textures.push_back({"normal", &normal});

  program->ApplyBindings(bindings);

  program->ApplyBindings(bindings);
}

TEST(vulkan_device_test, StandaloneDeviceAppliesProgramBufferBindings) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  const std::shared_ptr<cg::rhi::Program> program =
      cg::rhi::GetDevice().CreateProgram({
          .name = "synthetic_buffers",
          .kind = cg::rhi::ProgramKind::kRender,
      });
  ASSERT_TRUE(program != nullptr);

  ProbeBuffer material_buffer(cg::rhi::BufferType::kStorage);
  ProbeBuffer light_buffer(cg::rhi::BufferType::kStorage);

  cg::rhi::ProgramBindings bindings;
  bindings.buffers.push_back({
      .buffer = &material_buffer,
      .binding_point = 3,
  });
  bindings.buffers.push_back({
      .buffer = &light_buffer,
      .binding_point = 7,
  });

  program->ApplyBindings(bindings);

  ASSERT_EQ(material_buffer.bound_base_points().size(), 1u);
  EXPECT_EQ(material_buffer.bound_base_points()[0], 3u);
  ASSERT_EQ(light_buffer.bound_base_points().size(), 1u);
  EXPECT_EQ(light_buffer.bound_base_points()[0], 7u);
}

TEST(vulkan_device_test, StandaloneDeviceSupportsDefaultFramebufferRenderPassClear) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::rhi::RenderPassDesc render_pass_desc;
  render_pass_desc.framebuffer = 0;
  render_pass_desc.viewport_origin = {0, 0};
  render_pass_desc.viewport_size = {2, 2};
  render_pass_desc.clear_depth = true;
  render_pass_desc.clear_stencil = true;
  render_pass_desc.stencil_clear_value = 5;
  render_pass_desc.color_attachments.push_back({
      .attachment_index = 0,
      .clear = true,
      .clear_color = {0.0f, 1.0f, 0.0f, 1.0f},
  });

  cg::rhi::GetDevice().BeginRenderPass(render_pass_desc);
  const cg::rhi::FramebufferState framebuffer_state = cg::rhi::GetDevice().CaptureFramebufferState();
  EXPECT_EQ(framebuffer_state.framebuffer, 0);
  EXPECT_EQ(framebuffer_state.viewport, glm::ivec4(0, 0, 2, 2));
  cg::rhi::GetDevice().EndRenderPass();

  std::array<uint8_t, 16> readback = {};
  cg::rhi::ReadPixelsDesc default_readback_desc;
  default_readback_desc.framebuffer = 0;
  default_readback_desc.attachment_type = cg::rhi::AttachmentType::kColor;
  default_readback_desc.read_buffer = cg::rhi::ReadBuffer::kBack;
  default_readback_desc.origin = {0, 0};
  default_readback_desc.size = {2, 2};
  default_readback_desc.format = cg::rhi::PixelFormat::kRGBA;
  default_readback_desc.type = cg::rhi::PixelType::kUInt8;
  cg::rhi::GetDevice().ReadPixels(default_readback_desc, readback.data());
  for (size_t i = 0; i < readback.size(); i += 4) {
    EXPECT_EQ(readback[i + 0], 0u);
    EXPECT_EQ(readback[i + 1], 255u);
    EXPECT_EQ(readback[i + 2], 0u);
    EXPECT_EQ(readback[i + 3], 255u);
  }

  std::array<float, 4> depth_readback = {};
  cg::rhi::ReadPixelsDesc default_depth_readback_desc;
  default_depth_readback_desc.framebuffer = 0;
  default_depth_readback_desc.attachment_type = cg::rhi::AttachmentType::kDepth;
  default_depth_readback_desc.read_buffer = cg::rhi::ReadBuffer::kBack;
  default_depth_readback_desc.origin = {0, 0};
  default_depth_readback_desc.size = {2, 2};
  default_depth_readback_desc.format = cg::rhi::PixelFormat::kDepthComponent;
  default_depth_readback_desc.type = cg::rhi::PixelType::kFloat32;
  cg::rhi::GetDevice().ReadPixels(default_depth_readback_desc, depth_readback.data());
  for (float value : depth_readback) {
    EXPECT_FLOAT_EQ(value, 1.0f);
  }

  std::array<uint8_t, 4> stencil_readback = {};
  cg::rhi::ReadPixelsDesc default_stencil_readback_desc;
  default_stencil_readback_desc.framebuffer = 0;
  default_stencil_readback_desc.attachment_type = cg::rhi::AttachmentType::kStencil;
  default_stencil_readback_desc.read_buffer = cg::rhi::ReadBuffer::kBack;
  default_stencil_readback_desc.origin = {0, 0};
  default_stencil_readback_desc.size = {2, 2};
  default_stencil_readback_desc.format = cg::rhi::PixelFormat::kStencilIndex;
  default_stencil_readback_desc.type = cg::rhi::PixelType::kUInt8;
  cg::rhi::GetDevice().ReadPixels(default_stencil_readback_desc, stencil_readback.data());
  for (uint8_t value : stencil_readback) {
    EXPECT_EQ(value, 5u);
  }
}

TEST(vulkan_device_test, StandaloneDeviceSupportsDefaultFramebufferColorReadbackConversion) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::rhi::RenderPassDesc render_pass_desc;
  render_pass_desc.framebuffer = 0;
  render_pass_desc.viewport_size = {2, 2};
  render_pass_desc.color_attachments.push_back({
      .attachment_index = 0,
      .clear = true,
      .clear_color = {0.25f, 0.5f, 1.0f, 1.0f},
  });
  cg::rhi::GetDevice().BeginRenderPass(render_pass_desc);
  cg::rhi::GetDevice().EndRenderPass();

  std::array<float, 16> readback = {};
  cg::rhi::ReadPixelsDesc readback_desc;
  readback_desc.framebuffer = 0;
  readback_desc.attachment_type = cg::rhi::AttachmentType::kColor;
  readback_desc.read_buffer = cg::rhi::ReadBuffer::kBack;
  readback_desc.origin = {0, 0};
  readback_desc.size = {2, 2};
  readback_desc.format = cg::rhi::PixelFormat::kRGBA;
  readback_desc.type = cg::rhi::PixelType::kFloat32;
  cg::rhi::GetDevice().ReadPixels(readback_desc, readback.data());

  for (size_t i = 0; i < readback.size(); i += 4) {
    EXPECT_NEAR(readback[i + 0], 64.0f / 255.0f, 1e-6f);
    EXPECT_NEAR(readback[i + 1], 128.0f / 255.0f, 1e-6f);
    EXPECT_NEAR(readback[i + 2], 1.0f, 1e-6f);
    EXPECT_NEAR(readback[i + 3], 1.0f, 1e-6f);
  }
}

TEST(vulkan_device_test, StandaloneDeviceSeparatesDefaultFramebufferFrontAndBackColorBuffers) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::rhi::RenderPassDesc first_pass_desc;
  first_pass_desc.framebuffer = 0;
  first_pass_desc.viewport_size = {2, 2};
  first_pass_desc.color_attachments.push_back({
      .attachment_index = 0,
      .clear = true,
      .clear_color = {1.0f, 0.0f, 0.0f, 1.0f},
  });
  cg::rhi::GetDevice().BeginRenderPass(first_pass_desc);
  cg::rhi::GetDevice().EndRenderPass();

  std::array<uint8_t, 16> front_readback = {};
  cg::rhi::ReadPixelsDesc front_readback_desc;
  front_readback_desc.framebuffer = 0;
  front_readback_desc.attachment_type = cg::rhi::AttachmentType::kColor;
  front_readback_desc.read_buffer = cg::rhi::ReadBuffer::kFront;
  front_readback_desc.origin = {0, 0};
  front_readback_desc.size = {2, 2};
  front_readback_desc.format = cg::rhi::PixelFormat::kRGBA;
  front_readback_desc.type = cg::rhi::PixelType::kUInt8;
  cg::rhi::GetDevice().ReadPixels(front_readback_desc, front_readback.data());
  for (size_t i = 0; i < front_readback.size(); i += 4) {
    EXPECT_EQ(front_readback[i + 0], 255u);
    EXPECT_EQ(front_readback[i + 1], 0u);
    EXPECT_EQ(front_readback[i + 2], 0u);
    EXPECT_EQ(front_readback[i + 3], 255u);
  }

  cg::rhi::RenderPassDesc second_pass_desc;
  second_pass_desc.framebuffer = 0;
  second_pass_desc.viewport_size = {2, 2};
  second_pass_desc.color_attachments.push_back({
      .attachment_index = 0,
      .clear = true,
      .clear_color = {0.0f, 1.0f, 0.0f, 1.0f},
  });
  cg::rhi::GetDevice().BeginRenderPass(second_pass_desc);

  std::array<uint8_t, 16> back_readback = {};
  cg::rhi::ReadPixelsDesc back_readback_desc = front_readback_desc;
  back_readback_desc.read_buffer = cg::rhi::ReadBuffer::kBack;
  cg::rhi::GetDevice().ReadPixels(front_readback_desc, front_readback.data());
  cg::rhi::GetDevice().ReadPixels(back_readback_desc, back_readback.data());

  for (size_t i = 0; i < front_readback.size(); i += 4) {
    EXPECT_EQ(front_readback[i + 0], 255u);
    EXPECT_EQ(front_readback[i + 1], 0u);
    EXPECT_EQ(front_readback[i + 2], 0u);
    EXPECT_EQ(front_readback[i + 3], 255u);

    EXPECT_EQ(back_readback[i + 0], 0u);
    EXPECT_EQ(back_readback[i + 1], 255u);
    EXPECT_EQ(back_readback[i + 2], 0u);
    EXPECT_EQ(back_readback[i + 3], 255u);
  }

  cg::rhi::GetDevice().EndRenderPass();
  cg::rhi::GetDevice().ReadPixels(front_readback_desc, front_readback.data());
  for (size_t i = 0; i < front_readback.size(); i += 4) {
    EXPECT_EQ(front_readback[i + 0], 0u);
    EXPECT_EQ(front_readback[i + 1], 255u);
    EXPECT_EQ(front_readback[i + 2], 0u);
    EXPECT_EQ(front_readback[i + 3], 255u);
  }
}

TEST(vulkan_device_test, StandaloneDeviceSupportsDefaultFramebufferFrontAndBackSourceBlit) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::rhi::RenderPassDesc first_pass_desc;
  first_pass_desc.framebuffer = 0;
  first_pass_desc.viewport_size = {2, 2};
  first_pass_desc.color_attachments.push_back({
      .attachment_index = 0,
      .clear = true,
      .clear_color = {1.0f, 0.0f, 0.0f, 1.0f},
  });
  cg::rhi::GetDevice().BeginRenderPass(first_pass_desc);
  cg::rhi::GetDevice().EndRenderPass();

  cg::rhi::RenderPassDesc second_pass_desc;
  second_pass_desc.framebuffer = 0;
  second_pass_desc.viewport_size = {2, 2};
  second_pass_desc.color_attachments.push_back({
      .attachment_index = 0,
      .clear = true,
      .clear_color = {0.0f, 1.0f, 0.0f, 1.0f},
  });
  cg::rhi::GetDevice().BeginRenderPass(second_pass_desc);

  cg::Texture front_destination = MakeRgbaTexture(2, 2, {0u, 0u, 0u, 0u});
  cg::Texture back_destination = MakeRgbaTexture(2, 2, {0u, 0u, 0u, 0u});

  const uint32_t front_destination_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc front_destination_desc;
  front_destination_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &front_destination,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(front_destination_fbo, front_destination_desc));

  const uint32_t back_destination_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc back_destination_desc;
  back_destination_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &back_destination,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(back_destination_fbo, back_destination_desc));

  cg::rhi::BlitFramebufferDesc front_blit_desc;
  front_blit_desc.read_framebuffer = 0;
  front_blit_desc.draw_framebuffer = front_destination_fbo;
  front_blit_desc.read_buffer = cg::rhi::ReadBuffer::kFront;
  front_blit_desc.size = {2, 2};
  front_blit_desc.mask = cg::rhi::ClearMask::kColor;
  cg::rhi::GetDevice().BlitFramebuffer(front_blit_desc);

  cg::rhi::BlitFramebufferDesc back_blit_desc = front_blit_desc;
  back_blit_desc.draw_framebuffer = back_destination_fbo;
  back_blit_desc.read_buffer = cg::rhi::ReadBuffer::kBack;
  cg::rhi::GetDevice().BlitFramebuffer(back_blit_desc);

  cg::rhi::GetDevice().EndRenderPass();

  const std::vector<unsigned char> front_pixels = front_destination.GetData<unsigned char>();
  ASSERT_EQ(front_pixels.size(), 16u);
  for (size_t i = 0; i < front_pixels.size(); i += 4) {
    EXPECT_EQ(front_pixels[i + 0], 255u);
    EXPECT_EQ(front_pixels[i + 1], 0u);
    EXPECT_EQ(front_pixels[i + 2], 0u);
    EXPECT_EQ(front_pixels[i + 3], 255u);
  }

  const std::vector<unsigned char> back_pixels = back_destination.GetData<unsigned char>();
  ASSERT_EQ(back_pixels.size(), 16u);
  for (size_t i = 0; i < back_pixels.size(); i += 4) {
    EXPECT_EQ(back_pixels[i + 0], 0u);
    EXPECT_EQ(back_pixels[i + 1], 255u);
    EXPECT_EQ(back_pixels[i + 2], 0u);
    EXPECT_EQ(back_pixels[i + 3], 255u);
  }
}

TEST(vulkan_device_test, StandaloneDeviceSupportsDefaultFramebufferFrontAndBackDestinationBlit) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::rhi::RenderPassDesc first_pass_desc;
  first_pass_desc.framebuffer = 0;
  first_pass_desc.viewport_size = {2, 2};
  first_pass_desc.color_attachments.push_back({
      .attachment_index = 0,
      .clear = true,
      .clear_color = {1.0f, 0.0f, 0.0f, 1.0f},
  });
  cg::rhi::GetDevice().BeginRenderPass(first_pass_desc);
  cg::rhi::GetDevice().EndRenderPass();

  cg::Texture blue_source = MakeRgbaTexture(2, 2, {0u, 0u, 255u, 255u});
  const uint32_t source_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc source_desc;
  source_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &blue_source,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(source_fbo, source_desc));

  cg::rhi::BlitFramebufferDesc to_front_blit_desc;
  to_front_blit_desc.read_framebuffer = source_fbo;
  to_front_blit_desc.draw_framebuffer = 0;
  to_front_blit_desc.draw_buffer = cg::rhi::ReadBuffer::kFront;
  to_front_blit_desc.size = {2, 2};
  to_front_blit_desc.mask = cg::rhi::ClearMask::kColor;
  cg::rhi::GetDevice().BlitFramebuffer(to_front_blit_desc);

  std::array<uint8_t, 16> front_readback = {};
  cg::rhi::ReadPixelsDesc front_readback_desc;
  front_readback_desc.framebuffer = 0;
  front_readback_desc.attachment_type = cg::rhi::AttachmentType::kColor;
  front_readback_desc.read_buffer = cg::rhi::ReadBuffer::kFront;
  front_readback_desc.origin = {0, 0};
  front_readback_desc.size = {2, 2};
  front_readback_desc.format = cg::rhi::PixelFormat::kRGBA;
  front_readback_desc.type = cg::rhi::PixelType::kUInt8;
  cg::rhi::GetDevice().ReadPixels(front_readback_desc, front_readback.data());

  std::array<uint8_t, 16> back_readback = {};
  cg::rhi::ReadPixelsDesc back_readback_desc = front_readback_desc;
  back_readback_desc.read_buffer = cg::rhi::ReadBuffer::kBack;
  cg::rhi::GetDevice().ReadPixels(back_readback_desc, back_readback.data());

  for (size_t i = 0; i < front_readback.size(); i += 4) {
    EXPECT_EQ(front_readback[i + 0], 0u);
    EXPECT_EQ(front_readback[i + 1], 0u);
    EXPECT_EQ(front_readback[i + 2], 255u);
    EXPECT_EQ(front_readback[i + 3], 255u);

    EXPECT_EQ(back_readback[i + 0], 255u);
    EXPECT_EQ(back_readback[i + 1], 0u);
    EXPECT_EQ(back_readback[i + 2], 0u);
    EXPECT_EQ(back_readback[i + 3], 255u);
  }

  cg::rhi::RenderPassDesc second_pass_desc;
  second_pass_desc.framebuffer = 0;
  second_pass_desc.viewport_size = {2, 2};
  second_pass_desc.color_attachments.push_back({
      .attachment_index = 0,
      .clear = true,
      .clear_color = {0.0f, 1.0f, 0.0f, 1.0f},
  });
  cg::rhi::GetDevice().BeginRenderPass(second_pass_desc);

  cg::rhi::BlitFramebufferDesc default_to_default_blit_desc;
  default_to_default_blit_desc.read_framebuffer = 0;
  default_to_default_blit_desc.draw_framebuffer = 0;
  default_to_default_blit_desc.read_buffer = cg::rhi::ReadBuffer::kFront;
  default_to_default_blit_desc.draw_buffer = cg::rhi::ReadBuffer::kBack;
  default_to_default_blit_desc.size = {2, 2};
  default_to_default_blit_desc.mask = cg::rhi::ClearMask::kColor;
  cg::rhi::GetDevice().BlitFramebuffer(default_to_default_blit_desc);

  cg::rhi::GetDevice().ReadPixels(front_readback_desc, front_readback.data());
  cg::rhi::GetDevice().ReadPixels(back_readback_desc, back_readback.data());
  for (size_t i = 0; i < front_readback.size(); i += 4) {
    EXPECT_EQ(front_readback[i + 0], 0u);
    EXPECT_EQ(front_readback[i + 1], 0u);
    EXPECT_EQ(front_readback[i + 2], 255u);
    EXPECT_EQ(front_readback[i + 3], 255u);

    EXPECT_EQ(back_readback[i + 0], 0u);
    EXPECT_EQ(back_readback[i + 1], 0u);
    EXPECT_EQ(back_readback[i + 2], 255u);
    EXPECT_EQ(back_readback[i + 3], 255u);
  }

  cg::rhi::GetDevice().EndRenderPass();
}

TEST(vulkan_device_test, StandaloneDeviceSupportsOverlappingDefaultFramebufferSelfBlit) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::Texture source = MakePatternRgbaTexture(4, 4);
  const uint32_t source_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc source_desc;
  source_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &source,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(source_fbo, source_desc));

  cg::rhi::BlitFramebufferDesc to_default_blit_desc;
  to_default_blit_desc.read_framebuffer = source_fbo;
  to_default_blit_desc.draw_framebuffer = 0;
  to_default_blit_desc.size = {4, 4};
  to_default_blit_desc.mask = cg::rhi::ClearMask::kColor;
  cg::rhi::GetDevice().BlitFramebuffer(to_default_blit_desc);

  cg::rhi::BlitFramebufferDesc self_blit_desc;
  self_blit_desc.read_framebuffer = 0;
  self_blit_desc.draw_framebuffer = 0;
  self_blit_desc.read_buffer = cg::rhi::ReadBuffer::kBack;
  self_blit_desc.draw_buffer = cg::rhi::ReadBuffer::kBack;
  self_blit_desc.read_origin = {0, 0};
  self_blit_desc.draw_origin = {1, 1};
  self_blit_desc.size = {2, 2};
  self_blit_desc.mask = cg::rhi::ClearMask::kColor;
  cg::rhi::GetDevice().BlitFramebuffer(self_blit_desc);

  std::array<uint8_t, 64> readback = {};
  cg::rhi::ReadPixelsDesc readback_desc;
  readback_desc.framebuffer = 0;
  readback_desc.attachment_type = cg::rhi::AttachmentType::kColor;
  readback_desc.read_buffer = cg::rhi::ReadBuffer::kBack;
  readback_desc.origin = {0, 0};
  readback_desc.size = {4, 4};
  readback_desc.format = cg::rhi::PixelFormat::kRGBA;
  readback_desc.type = cg::rhi::PixelType::kUInt8;
  cg::rhi::GetDevice().ReadPixels(readback_desc, readback.data());

  for (int row = 0; row < 4; ++row) {
    for (int column = 0; column < 4; ++column) {
      const std::array<uint8_t, 4> expected =
          (row >= 1 && row < 3 && column >= 1 && column < 3) ? PatternPixel(row - 1, column - 1) : PatternPixel(row, column);
      const size_t offset = static_cast<size_t>(row * 4 + column) * 4;
      EXPECT_EQ(readback[offset + 0], expected[0]);
      EXPECT_EQ(readback[offset + 1], expected[1]);
      EXPECT_EQ(readback[offset + 2], expected[2]);
      EXPECT_EQ(readback[offset + 3], expected[3]);
    }
  }
}

TEST(vulkan_device_test, StandaloneDeviceSupportsOverlappingAttachmentSelfBlit) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::Texture texture = MakePatternRgbaTexture(4, 4);
  const uint32_t framebuffer = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc framebuffer_desc;
  framebuffer_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &texture,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(framebuffer, framebuffer_desc));

  cg::rhi::BlitFramebufferDesc self_blit_desc;
  self_blit_desc.read_framebuffer = framebuffer;
  self_blit_desc.draw_framebuffer = framebuffer;
  self_blit_desc.read_origin = {0, 0};
  self_blit_desc.draw_origin = {1, 1};
  self_blit_desc.size = {2, 2};
  self_blit_desc.mask = cg::rhi::ClearMask::kColor;
  cg::rhi::GetDevice().BlitFramebuffer(self_blit_desc);

  const std::vector<unsigned char> pixels = texture.GetData<unsigned char>();
  ASSERT_EQ(pixels.size(), 64u);
  for (int row = 0; row < 4; ++row) {
    for (int column = 0; column < 4; ++column) {
      const std::array<uint8_t, 4> expected =
          (row >= 1 && row < 3 && column >= 1 && column < 3) ? PatternPixel(row - 1, column - 1) : PatternPixel(row, column);
      const size_t offset = static_cast<size_t>(row * 4 + column) * 4;
      EXPECT_EQ(pixels[offset + 0], expected[0]);
      EXPECT_EQ(pixels[offset + 1], expected[1]);
      EXPECT_EQ(pixels[offset + 2], expected[2]);
      EXPECT_EQ(pixels[offset + 3], expected[3]);
    }
  }
}

TEST(vulkan_device_test, StandaloneDeviceRespectsBlitSizeWhenCopyingDefaultFramebufferToAttachments) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::rhi::RenderPassDesc render_pass_desc;
  render_pass_desc.framebuffer = 0;
  render_pass_desc.viewport_size = {4, 4};
  render_pass_desc.clear_depth = true;
  render_pass_desc.depth_clear_value = 0.3f;
  render_pass_desc.clear_stencil = true;
  render_pass_desc.stencil_clear_value = 11;
  render_pass_desc.color_attachments.push_back({
      .attachment_index = 0,
      .clear = true,
      .clear_color = {1.0f, 0.0f, 0.0f, 1.0f},
  });
  cg::rhi::GetDevice().BeginRenderPass(render_pass_desc);
  cg::rhi::GetDevice().EndRenderPass();

  cg::Texture destination_color = MakeRgbaTexture(4, 4, {0u, 0u, 0u, 0u});
  cg::Texture destination_depth = MakeDepthTexture(4, 4, 1.0f);
  cg::Texture destination_stencil = MakeStencilTexture(4, 4, 0u);
  const uint32_t destination_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc destination_desc;
  destination_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &destination_color,
  });
  destination_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kDepth,
      .attachment_index = 0,
      .texture = &destination_depth,
  });
  destination_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kStencil,
      .attachment_index = 0,
      .texture = &destination_stencil,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(destination_fbo, destination_desc));

  cg::rhi::BlitFramebufferDesc blit_desc;
  blit_desc.read_framebuffer = 0;
  blit_desc.draw_framebuffer = destination_fbo;
  blit_desc.size = {2, 2};
  blit_desc.mask = cg::rhi::ClearMask::kColor | cg::rhi::ClearMask::kDepth | cg::rhi::ClearMask::kStencil;
  blit_desc.filter = cg::rhi::FilterMode::kNearest;
  cg::rhi::GetDevice().BlitFramebuffer(blit_desc);

  const std::vector<uint8_t> color_values = destination_color.GetData<uint8_t>();
  const std::vector<float> depth_values = destination_depth.GetData<float>();
  const std::vector<uint8_t> stencil_values = destination_stencil.GetData<uint8_t>();
  ASSERT_EQ(color_values.size(), 64u);
  ASSERT_EQ(depth_values.size(), 16u);
  ASSERT_EQ(stencil_values.size(), 16u);
  for (int row = 0; row < 4; ++row) {
    for (int column = 0; column < 4; ++column) {
      const bool copied = row < 2 && column < 2;
      const size_t color_offset = static_cast<size_t>(row * 4 + column) * 4;
      EXPECT_EQ(color_values[color_offset + 0], copied ? 255u : 0u);
      EXPECT_EQ(color_values[color_offset + 1], 0u);
      EXPECT_EQ(color_values[color_offset + 2], 0u);
      EXPECT_EQ(color_values[color_offset + 3], copied ? 255u : 0u);
      EXPECT_FLOAT_EQ(depth_values[static_cast<size_t>(row * 4 + column)], copied ? 0.3f : 1.0f);
      EXPECT_EQ(stencil_values[static_cast<size_t>(row * 4 + column)], copied ? 11u : 0u);
    }
  }
}

TEST(vulkan_device_test, StandaloneDeviceSupportsBufferFramebufferAndReadbackState) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  std::unique_ptr<cg::rhi::Buffer> buffer = cg::rhi::GetDevice().CreateBuffer(cg::rhi::BufferType::kStorage);
  ASSERT_TRUE(buffer != nullptr);
  const std::array<uint32_t, 4> initial_values = {1u, 2u, 3u, 4u};
  buffer->SetData(sizeof(initial_values), initial_values.data(), cg::rhi::BufferUsage::kStatic);
  const uint32_t replacement_value = 42u;
  buffer->UpdateData(sizeof(uint32_t), sizeof(replacement_value), &replacement_value);
  std::array<uint32_t, 4> readback_words = {};
  buffer->ReadData({
      .offset_in_bytes = 0,
      .size_in_bytes = sizeof(readback_words),
  }, readback_words.data());
  EXPECT_EQ(readback_words[0], 1u);
  EXPECT_EQ(readback_words[1], 42u);
  EXPECT_EQ(readback_words[2], 3u);
  EXPECT_EQ(readback_words[3], 4u);

  cg::Texture source = MakeRgbaTexture(2, 2, {0u, 0u, 0u, 0u});
  cg::Texture source_depth = MakeDepthTexture(2, 2, 1.0f);
  cg::Texture destination = MakeRgbaTexture(2, 2, {0u, 0u, 0u, 0u});

  const uint32_t source_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc source_framebuffer_desc;
  source_framebuffer_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &source,
  });
  source_framebuffer_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kDepth,
      .attachment_index = 0,
      .texture = &source_depth,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(source_fbo, source_framebuffer_desc));
  cg::rhi::RenderPassDesc source_render_pass_desc;
  source_render_pass_desc.framebuffer = source_fbo;
  source_render_pass_desc.viewport_size = {2, 2};
  source_render_pass_desc.clear_depth = true;
  source_render_pass_desc.color_attachments.push_back({
      .attachment_index = 0,
      .clear = true,
      .clear_color = {1.0f, 0.0f, 0.0f, 1.0f},
  });
  cg::rhi::GetDevice().BeginRenderPass(source_render_pass_desc);
  cg::rhi::FramebufferState framebuffer_state = cg::rhi::GetDevice().CaptureFramebufferState();
  EXPECT_EQ(framebuffer_state.framebuffer, static_cast<int32_t>(source_fbo));
  EXPECT_EQ(framebuffer_state.viewport, glm::ivec4(0, 0, 2, 2));
  cg::rhi::GetDevice().EndRenderPass();

  const std::vector<unsigned char> source_pixels = source.GetData<unsigned char>();
  ASSERT_EQ(source_pixels.size(), 16u);
  for (size_t i = 0; i < source_pixels.size(); i += 4) {
    EXPECT_EQ(source_pixels[i + 0], 255u);
    EXPECT_EQ(source_pixels[i + 1], 0u);
    EXPECT_EQ(source_pixels[i + 2], 0u);
    EXPECT_EQ(source_pixels[i + 3], 255u);
  }

  const uint32_t destination_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc destination_framebuffer_desc;
  destination_framebuffer_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &destination,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(destination_fbo, destination_framebuffer_desc));

  cg::rhi::BlitFramebufferDesc to_default_blit_desc;
  to_default_blit_desc.read_framebuffer = source_fbo;
  to_default_blit_desc.draw_framebuffer = 0;
  to_default_blit_desc.size = {2, 2};
  to_default_blit_desc.mask = cg::rhi::ClearMask::kColor;
  to_default_blit_desc.filter = cg::rhi::FilterMode::kNearest;
  cg::rhi::GetDevice().BlitFramebuffer(to_default_blit_desc);
  std::array<uint8_t, 16> default_readback = {};
  cg::rhi::ReadPixelsDesc blit_default_readback_desc;
  blit_default_readback_desc.framebuffer = 0;
  blit_default_readback_desc.attachment_type = cg::rhi::AttachmentType::kColor;
  blit_default_readback_desc.read_buffer = cg::rhi::ReadBuffer::kBack;
  blit_default_readback_desc.origin = {0, 0};
  blit_default_readback_desc.size = {2, 2};
  blit_default_readback_desc.format = cg::rhi::PixelFormat::kRGBA;
  blit_default_readback_desc.type = cg::rhi::PixelType::kUInt8;
  cg::rhi::GetDevice().ReadPixels(blit_default_readback_desc, default_readback.data());
  EXPECT_TRUE(std::equal(default_readback.begin(), default_readback.end(), source_pixels.begin(), source_pixels.end()));

  cg::rhi::BlitFramebufferDesc to_destination_blit_desc;
  to_destination_blit_desc.read_framebuffer = 0;
  to_destination_blit_desc.draw_framebuffer = destination_fbo;
  to_destination_blit_desc.size = {2, 2};
  to_destination_blit_desc.mask = cg::rhi::ClearMask::kColor;
  to_destination_blit_desc.filter = cg::rhi::FilterMode::kNearest;
  cg::rhi::GetDevice().BlitFramebuffer(to_destination_blit_desc);
  std::array<uint8_t, 16> readback = {};
  cg::rhi::ReadPixelsDesc destination_readback_desc;
  destination_readback_desc.framebuffer = destination_fbo;
  destination_readback_desc.attachment_type = cg::rhi::AttachmentType::kColor;
  destination_readback_desc.attachment_index = 0;
  destination_readback_desc.origin = {0, 0};
  destination_readback_desc.size = {2, 2};
  destination_readback_desc.format = cg::rhi::PixelFormat::kRGBA;
  destination_readback_desc.type = cg::rhi::PixelType::kUInt8;
  cg::rhi::GetDevice().ReadPixels(destination_readback_desc, readback.data());
  EXPECT_TRUE(std::equal(readback.begin(), readback.end(), source_pixels.begin(), source_pixels.end()));

  const std::vector<unsigned char> destination_pixels = destination.GetData<unsigned char>();
  EXPECT_TRUE(std::equal(destination_pixels.begin(), destination_pixels.end(),
                         source_pixels.begin(), source_pixels.end()));
}

TEST(vulkan_device_test, StandaloneDeviceUsesDrawFramebufferForWriteState) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::Texture source = MakeRgbaTexture(2, 2, {0u, 0u, 0u, 0u});
  cg::Texture destination = MakeRgbaTexture(2, 2, {0u, 0u, 0u, 0u});

  const uint32_t source_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc source_desc;
  source_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &source,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(source_fbo, source_desc));

  const uint32_t destination_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc destination_desc;
  destination_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &destination,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(destination_fbo, destination_desc));

  cg::rhi::GetDevice().RestoreFramebufferState({
      .read_framebuffer = static_cast<int32_t>(source_fbo),
      .draw_framebuffer = static_cast<int32_t>(destination_fbo),
      .framebuffer = static_cast<int32_t>(destination_fbo),
      .viewport = {0, 0, 2, 2},
  });

  const cg::rhi::FramebufferState framebuffer_state = cg::rhi::GetDevice().CaptureFramebufferState();
  EXPECT_EQ(framebuffer_state.framebuffer, static_cast<int32_t>(destination_fbo));

  cg::rhi::ClearDesc clear_desc;
  clear_desc.mask = cg::rhi::ClearMask::kColor;
  clear_desc.clear_color = {0.0f, 0.0f, 1.0f, 1.0f};
  cg::rhi::GetDevice().Clear(clear_desc);

  const std::vector<unsigned char> source_pixels = source.GetData<unsigned char>();
  ASSERT_EQ(source_pixels.size(), 16u);
  for (size_t i = 0; i < source_pixels.size(); i += 4) {
    EXPECT_EQ(source_pixels[i + 0], 0u);
    EXPECT_EQ(source_pixels[i + 1], 0u);
    EXPECT_EQ(source_pixels[i + 2], 0u);
    EXPECT_EQ(source_pixels[i + 3], 0u);
  }

  const std::vector<unsigned char> destination_pixels = destination.GetData<unsigned char>();
  ASSERT_EQ(destination_pixels.size(), 16u);
  for (size_t i = 0; i < destination_pixels.size(); i += 4) {
    EXPECT_EQ(destination_pixels[i + 0], 0u);
    EXPECT_EQ(destination_pixels[i + 1], 0u);
    EXPECT_EQ(destination_pixels[i + 2], 255u);
    EXPECT_EQ(destination_pixels[i + 3], 255u);
  }
}

TEST(vulkan_device_test, StandaloneDeviceCapturesAndRestoresRenderState) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::rhi::GetDevice().ApplyRenderState({
      .depth_test_enabled = true,
      .cull_enabled = true,
      .cull_mode = cg::rhi::CullMode::kFront,
      .front_face = cg::rhi::FrontFace::kClockwise,
  });

  const cg::rhi::RenderState initial_state = cg::rhi::GetDevice().CaptureRenderState();
  EXPECT_TRUE(initial_state.depth_test_enabled);
  EXPECT_TRUE(initial_state.cull_enabled);
  EXPECT_EQ(initial_state.cull_mode, cg::rhi::CullMode::kFront);
  EXPECT_EQ(initial_state.front_face, cg::rhi::FrontFace::kClockwise);

  cg::rhi::GetDevice().ApplyRenderState({
      .depth_test_enabled = false,
      .cull_enabled = false,
      .cull_mode = cg::rhi::CullMode::kBack,
      .front_face = cg::rhi::FrontFace::kCounterClockwise,
  });

  cg::rhi::GetDevice().RestoreRenderState(initial_state);

  const cg::rhi::RenderState restored_state = cg::rhi::GetDevice().CaptureRenderState();
  EXPECT_TRUE(restored_state.depth_test_enabled);
  EXPECT_TRUE(restored_state.cull_enabled);
  EXPECT_EQ(restored_state.cull_mode, cg::rhi::CullMode::kFront);
  EXPECT_EQ(restored_state.front_face, cg::rhi::FrontFace::kClockwise);
}

TEST(vulkan_device_test, StandaloneDeviceRejectsReadPixelsOnUnsupportedDefaultAttachmentIndex) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  EXPECT_DEATH(
      ([] {
        auto device = cg::rhi::CreateVulkanDevice(false);
        if (device == nullptr) {
          std::abort();
        }
        cg::rhi::SetDevice(std::move(device));
        std::array<unsigned char, 4> readback = {};
        cg::rhi::ReadPixelsDesc readback_desc;
        readback_desc.framebuffer = 0;
        readback_desc.attachment_type = cg::rhi::AttachmentType::kColor;
        readback_desc.attachment_index = 1;
        readback_desc.size = {1, 1};
        cg::rhi::GetDevice().ReadPixels(readback_desc, readback.data());
      }()),
      ".*");
}

TEST(vulkan_device_test, StandaloneDeviceRejectsReadPixelsOnMissingNonDefaultAttachment) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  EXPECT_DEATH(
      ([] {
        auto device = cg::rhi::CreateVulkanDevice(false);
        if (device == nullptr) {
          std::abort();
        }
        cg::rhi::SetDevice(std::move(device));

        cg::Texture color = MakeRgbaTexture(2, 2, {0u, 0u, 0u, 0u});
        const uint32_t framebuffer = cg::rhi::GetDevice().CreateFramebuffer();
        cg::rhi::FramebufferDesc framebuffer_desc;
        framebuffer_desc.attachments.push_back({
            .attachment_type = cg::rhi::AttachmentType::kColor,
            .attachment_index = 0,
            .texture = &color,
        });
        if (!cg::rhi::GetDevice().ConfigureFramebuffer(framebuffer, framebuffer_desc)) {
          std::abort();
        }

        std::array<unsigned char, 4> readback = {};
        cg::rhi::ReadPixelsDesc readback_desc;
        readback_desc.framebuffer = framebuffer;
        readback_desc.attachment_type = cg::rhi::AttachmentType::kColor;
        readback_desc.attachment_index = 1;
        readback_desc.size = {1, 1};
        cg::rhi::GetDevice().ReadPixels(readback_desc, readback.data());
      }()),
      ".*");
}

TEST(vulkan_device_test, StandaloneDeviceRejectsBlitOnUnsupportedDefaultColorAttachmentIndex) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  EXPECT_DEATH(
      ([] {
        auto device = cg::rhi::CreateVulkanDevice(false);
        if (device == nullptr) {
          std::abort();
        }
        cg::rhi::SetDevice(std::move(device));

        cg::rhi::BlitFramebufferDesc blit_desc;
        blit_desc.read_framebuffer = 0;
        blit_desc.draw_framebuffer = 0;
        blit_desc.read_color_attachment_index = 1;
        blit_desc.size = {1, 1};
        blit_desc.mask = cg::rhi::ClearMask::kColor;
        cg::rhi::GetDevice().BlitFramebuffer(blit_desc);
      }()),
      ".*");
}

TEST(vulkan_device_test, StandaloneDeviceRejectsBlitOnMissingNonDefaultColorAttachment) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  EXPECT_DEATH(
      ([] {
        auto device = cg::rhi::CreateVulkanDevice(false);
        if (device == nullptr) {
          std::abort();
        }
        cg::rhi::SetDevice(std::move(device));

        cg::Texture source = MakeRgbaTexture(2, 2, {255u, 0u, 0u, 255u});
        const uint32_t framebuffer = cg::rhi::GetDevice().CreateFramebuffer();
        cg::rhi::FramebufferDesc framebuffer_desc;
        framebuffer_desc.attachments.push_back({
            .attachment_type = cg::rhi::AttachmentType::kColor,
            .attachment_index = 0,
            .texture = &source,
        });
        if (!cg::rhi::GetDevice().ConfigureFramebuffer(framebuffer, framebuffer_desc)) {
          std::abort();
        }

        cg::rhi::BlitFramebufferDesc blit_desc;
        blit_desc.read_framebuffer = framebuffer;
        blit_desc.draw_framebuffer = 0;
        blit_desc.read_color_attachment_index = 1;
        blit_desc.size = {1, 1};
        blit_desc.mask = cg::rhi::ClearMask::kColor;
        cg::rhi::GetDevice().BlitFramebuffer(blit_desc);
      }()),
      ".*");
}

TEST(vulkan_device_test, StandaloneDeviceRejectsBlitOnMissingNonDefaultDestinationColorAttachment) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  EXPECT_DEATH(
      ([] {
        auto device = cg::rhi::CreateVulkanDevice(false);
        if (device == nullptr) {
          std::abort();
        }
        cg::rhi::SetDevice(std::move(device));

        cg::Texture destination = MakeRgbaTexture(2, 2, {0u, 0u, 0u, 0u});
        const uint32_t framebuffer = cg::rhi::GetDevice().CreateFramebuffer();
        cg::rhi::FramebufferDesc framebuffer_desc;
        framebuffer_desc.attachments.push_back({
            .attachment_type = cg::rhi::AttachmentType::kColor,
            .attachment_index = 0,
            .texture = &destination,
        });
        if (!cg::rhi::GetDevice().ConfigureFramebuffer(framebuffer, framebuffer_desc)) {
          std::abort();
        }

        cg::rhi::BlitFramebufferDesc blit_desc;
        blit_desc.read_framebuffer = 0;
        blit_desc.draw_framebuffer = framebuffer;
        blit_desc.draw_color_attachment_index = 1;
        blit_desc.size = {1, 1};
        blit_desc.mask = cg::rhi::ClearMask::kColor;
        cg::rhi::GetDevice().BlitFramebuffer(blit_desc);
      }()),
      ".*");
}

TEST(vulkan_device_test, StandaloneDeviceCapturesAndRestoresSplitFramebufferState) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  const uint32_t read_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  const uint32_t draw_fbo = cg::rhi::GetDevice().CreateFramebuffer();

  cg::rhi::GetDevice().RestoreFramebufferState({
      .read_framebuffer = static_cast<int32_t>(read_fbo),
      .draw_framebuffer = static_cast<int32_t>(draw_fbo),
      .framebuffer = static_cast<int32_t>(draw_fbo),
      .viewport = {4, 5, 6, 7},
  });

  const cg::rhi::FramebufferState captured_state = cg::rhi::GetDevice().CaptureFramebufferState();
  EXPECT_EQ(captured_state.read_framebuffer, static_cast<int32_t>(read_fbo));
  EXPECT_EQ(captured_state.draw_framebuffer, static_cast<int32_t>(draw_fbo));
  EXPECT_EQ(captured_state.framebuffer, static_cast<int32_t>(draw_fbo));
  EXPECT_EQ(captured_state.viewport, glm::ivec4(4, 5, 6, 7));

  cg::rhi::GetDevice().RestoreFramebufferState({
      .read_framebuffer = 0,
      .draw_framebuffer = 0,
      .framebuffer = 0,
      .viewport = {0, 0, 1, 1},
  });
  cg::rhi::GetDevice().RestoreFramebufferState(captured_state);

  const cg::rhi::FramebufferState restored_state = cg::rhi::GetDevice().CaptureFramebufferState();
  EXPECT_EQ(restored_state.read_framebuffer, static_cast<int32_t>(read_fbo));
  EXPECT_EQ(restored_state.draw_framebuffer, static_cast<int32_t>(draw_fbo));
  EXPECT_EQ(restored_state.framebuffer, static_cast<int32_t>(draw_fbo));
  EXPECT_EQ(restored_state.viewport, glm::ivec4(4, 5, 6, 7));
}

TEST(vulkan_device_test, StandaloneDeviceSupportsOffsetAttachmentBlit) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::Texture source = MakePatternRgbaTexture(4, 4);
  cg::Texture destination = MakeRgbaTexture(4, 4, {0u, 0u, 0u, 0u});

  const uint32_t source_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc source_desc;
  source_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &source,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(source_fbo, source_desc));

  const uint32_t destination_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc destination_desc;
  destination_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &destination,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(destination_fbo, destination_desc));

  cg::rhi::BlitFramebufferDesc blit_desc;
  blit_desc.read_framebuffer = source_fbo;
  blit_desc.draw_framebuffer = destination_fbo;
  blit_desc.read_origin = {1, 1};
  blit_desc.draw_origin = {0, 2};
  blit_desc.size = {2, 2};
  blit_desc.mask = cg::rhi::ClearMask::kColor;
  blit_desc.filter = cg::rhi::FilterMode::kNearest;
  cg::rhi::GetDevice().BlitFramebuffer(blit_desc);

  const std::vector<unsigned char> destination_pixels = destination.GetData<unsigned char>();
  ASSERT_EQ(destination_pixels.size(), 64u);
  for (int row = 0; row < 4; ++row) {
    for (int column = 0; column < 4; ++column) {
      const size_t offset = static_cast<size_t>(row * 4 + column) * 4;
      if (row >= 2 && row < 4 && column < 2) {
        const std::array<uint8_t, 4> expected = PatternPixel(row - 1, column + 1);
        EXPECT_EQ(destination_pixels[offset + 0], expected[0]);
        EXPECT_EQ(destination_pixels[offset + 1], expected[1]);
        EXPECT_EQ(destination_pixels[offset + 2], expected[2]);
        EXPECT_EQ(destination_pixels[offset + 3], expected[3]);
      } else {
        EXPECT_EQ(destination_pixels[offset + 0], 0u);
        EXPECT_EQ(destination_pixels[offset + 1], 0u);
        EXPECT_EQ(destination_pixels[offset + 2], 0u);
        EXPECT_EQ(destination_pixels[offset + 3], 0u);
      }
    }
  }
}

TEST(vulkan_device_test, StandaloneDeviceSupportsScaledAttachmentBlit) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::Texture source = MakePatternRgbaTexture(2, 2);
  cg::Texture destination = MakeRgbaTexture(4, 4, {0u, 0u, 0u, 0u});

  const uint32_t source_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc source_desc;
  source_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &source,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(source_fbo, source_desc));

  const uint32_t destination_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc destination_desc;
  destination_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &destination,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(destination_fbo, destination_desc));

  cg::rhi::BlitFramebufferDesc blit_desc;
  blit_desc.read_framebuffer = source_fbo;
  blit_desc.draw_framebuffer = destination_fbo;
  blit_desc.size = {2, 2};
  blit_desc.draw_size = {4, 4};
  blit_desc.mask = cg::rhi::ClearMask::kColor;
  blit_desc.filter = cg::rhi::FilterMode::kNearest;
  cg::rhi::GetDevice().BlitFramebuffer(blit_desc);

  const std::vector<unsigned char> destination_pixels = destination.GetData<unsigned char>();
  ASSERT_EQ(destination_pixels.size(), 64u);
  for (int row = 0; row < 4; ++row) {
    for (int column = 0; column < 4; ++column) {
      const std::array<uint8_t, 4> expected = PatternPixel(row / 2, column / 2);
      const size_t offset = static_cast<size_t>(row * 4 + column) * 4;
      EXPECT_EQ(destination_pixels[offset + 0], expected[0]);
      EXPECT_EQ(destination_pixels[offset + 1], expected[1]);
      EXPECT_EQ(destination_pixels[offset + 2], expected[2]);
      EXPECT_EQ(destination_pixels[offset + 3], expected[3]);
    }
  }
}

TEST(vulkan_device_test, StandaloneDeviceClipsScaledAttachmentBlitWhenDestinationRectExceedsBounds) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::Texture source = MakePatternRgbaTexture(4, 4);
  cg::Texture destination = MakeRgbaTexture(4, 4, {0u, 0u, 0u, 0u});

  const uint32_t source_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc source_desc;
  source_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &source,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(source_fbo, source_desc));

  const uint32_t destination_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc destination_desc;
  destination_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &destination,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(destination_fbo, destination_desc));

  cg::rhi::BlitFramebufferDesc blit_desc;
  blit_desc.read_framebuffer = source_fbo;
  blit_desc.draw_framebuffer = destination_fbo;
  blit_desc.size = {4, 4};
  blit_desc.draw_size = {8, 8};
  blit_desc.mask = cg::rhi::ClearMask::kColor;
  blit_desc.filter = cg::rhi::FilterMode::kNearest;
  cg::rhi::GetDevice().BlitFramebuffer(blit_desc);

  const std::vector<unsigned char> destination_pixels = destination.GetData<unsigned char>();
  ASSERT_EQ(destination_pixels.size(), 64u);
  for (int row = 0; row < 4; ++row) {
    for (int column = 0; column < 4; ++column) {
      const std::array<uint8_t, 4> expected = PatternPixel(row / 2, column / 2);
      const size_t offset = static_cast<size_t>(row * 4 + column) * 4;
      EXPECT_EQ(destination_pixels[offset + 0], expected[0]);
      EXPECT_EQ(destination_pixels[offset + 1], expected[1]);
      EXPECT_EQ(destination_pixels[offset + 2], expected[2]);
      EXPECT_EQ(destination_pixels[offset + 3], expected[3]);
    }
  }
}

TEST(vulkan_device_test, StandaloneDeviceSupportsNegativeDestinationOriginAttachmentBlit) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::Texture source = MakePatternRgbaTexture(4, 4);
  cg::Texture destination = MakeRgbaTexture(4, 4, {0u, 0u, 0u, 0u});

  const uint32_t source_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc source_desc;
  source_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &source,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(source_fbo, source_desc));

  const uint32_t destination_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc destination_desc;
  destination_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &destination,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(destination_fbo, destination_desc));

  cg::rhi::BlitFramebufferDesc blit_desc;
  blit_desc.read_framebuffer = source_fbo;
  blit_desc.draw_framebuffer = destination_fbo;
  blit_desc.draw_origin = {-2, -2};
  blit_desc.size = {4, 4};
  blit_desc.draw_size = {4, 4};
  blit_desc.mask = cg::rhi::ClearMask::kColor;
  blit_desc.filter = cg::rhi::FilterMode::kNearest;
  cg::rhi::GetDevice().BlitFramebuffer(blit_desc);

  const std::vector<unsigned char> destination_pixels = destination.GetData<unsigned char>();
  ASSERT_EQ(destination_pixels.size(), 64u);
  for (int row = 0; row < 4; ++row) {
    for (int column = 0; column < 4; ++column) {
      const size_t offset = static_cast<size_t>(row * 4 + column) * 4;
      if (row < 2 && column < 2) {
        const std::array<uint8_t, 4> expected = PatternPixel(row + 2, column + 2);
        EXPECT_EQ(destination_pixels[offset + 0], expected[0]);
        EXPECT_EQ(destination_pixels[offset + 1], expected[1]);
        EXPECT_EQ(destination_pixels[offset + 2], expected[2]);
        EXPECT_EQ(destination_pixels[offset + 3], expected[3]);
      } else {
        EXPECT_EQ(destination_pixels[offset + 0], 0u);
        EXPECT_EQ(destination_pixels[offset + 1], 0u);
        EXPECT_EQ(destination_pixels[offset + 2], 0u);
        EXPECT_EQ(destination_pixels[offset + 3], 0u);
      }
    }
  }
}

TEST(vulkan_device_test, StandaloneDeviceSupportsLinearScaledAttachmentBlit) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::Texture source = MakeCornerRgbaFloatTexture();
  cg::Texture destination = MakeRgbaFloatTexture(3, 3, {0.0f, 0.0f, 0.0f, 0.0f});

  const uint32_t source_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc source_desc;
  source_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &source,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(source_fbo, source_desc));

  const uint32_t destination_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc destination_desc;
  destination_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &destination,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(destination_fbo, destination_desc));

  cg::rhi::BlitFramebufferDesc blit_desc;
  blit_desc.read_framebuffer = source_fbo;
  blit_desc.draw_framebuffer = destination_fbo;
  blit_desc.size = {2, 2};
  blit_desc.draw_size = {3, 3};
  blit_desc.mask = cg::rhi::ClearMask::kColor;
  blit_desc.filter = cg::rhi::FilterMode::kLinear;
  cg::rhi::GetDevice().BlitFramebuffer(blit_desc);

  const std::vector<float> destination_pixels = destination.GetData<float>();
  ASSERT_EQ(destination_pixels.size(), 36u);

  const size_t center_offset = static_cast<size_t>(1 * 3 + 1) * 4;
  EXPECT_NEAR(destination_pixels[center_offset + 0], 0.5f, 1e-6f);
  EXPECT_NEAR(destination_pixels[center_offset + 1], 0.5f, 1e-6f);
  EXPECT_NEAR(destination_pixels[center_offset + 2], 0.0f, 1e-6f);
  EXPECT_NEAR(destination_pixels[center_offset + 3], 1.0f, 1e-6f);

  const size_t top_left_offset = 0;
  EXPECT_NEAR(destination_pixels[top_left_offset + 0], 0.0f, 1e-6f);
  EXPECT_NEAR(destination_pixels[top_left_offset + 1], 0.0f, 1e-6f);
  EXPECT_NEAR(destination_pixels[top_left_offset + 2], 0.0f, 1e-6f);
  EXPECT_NEAR(destination_pixels[top_left_offset + 3], 1.0f, 1e-6f);

  const size_t bottom_right_offset = static_cast<size_t>(2 * 3 + 2) * 4;
  EXPECT_NEAR(destination_pixels[bottom_right_offset + 0], 1.0f, 1e-6f);
  EXPECT_NEAR(destination_pixels[bottom_right_offset + 1], 1.0f, 1e-6f);
  EXPECT_NEAR(destination_pixels[bottom_right_offset + 2], 0.0f, 1e-6f);
  EXPECT_NEAR(destination_pixels[bottom_right_offset + 3], 1.0f, 1e-6f);
}

TEST(vulkan_device_test, StandaloneDeviceSupportsScaledAttachmentToDefaultFramebufferBlit) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::Texture source = MakePatternRgbaTexture(2, 2);
  const uint32_t source_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc source_desc;
  source_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &source,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(source_fbo, source_desc));

  cg::rhi::BlitFramebufferDesc blit_desc;
  blit_desc.read_framebuffer = source_fbo;
  blit_desc.draw_framebuffer = 0;
  blit_desc.size = {2, 2};
  blit_desc.draw_size = {4, 4};
  blit_desc.mask = cg::rhi::ClearMask::kColor;
  blit_desc.filter = cg::rhi::FilterMode::kNearest;
  cg::rhi::GetDevice().BlitFramebuffer(blit_desc);

  std::array<uint8_t, 64> readback = {};
  cg::rhi::ReadPixelsDesc readback_desc;
  readback_desc.framebuffer = 0;
  readback_desc.attachment_type = cg::rhi::AttachmentType::kColor;
  readback_desc.read_buffer = cg::rhi::ReadBuffer::kBack;
  readback_desc.origin = {0, 0};
  readback_desc.size = {4, 4};
  readback_desc.format = cg::rhi::PixelFormat::kRGBA;
  readback_desc.type = cg::rhi::PixelType::kUInt8;
  cg::rhi::GetDevice().ReadPixels(readback_desc, readback.data());

  for (int row = 0; row < 4; ++row) {
    for (int column = 0; column < 4; ++column) {
      const std::array<uint8_t, 4> expected = PatternPixel(row / 2, column / 2);
      const size_t offset = static_cast<size_t>(row * 4 + column) * 4;
      EXPECT_EQ(readback[offset + 0], expected[0]);
      EXPECT_EQ(readback[offset + 1], expected[1]);
      EXPECT_EQ(readback[offset + 2], expected[2]);
      EXPECT_EQ(readback[offset + 3], expected[3]);
    }
  }
}

TEST(vulkan_device_test, StandaloneDeviceSupportsNegativeDestinationOriginOnUninitializedDefaultFramebufferBlit) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::Texture source = MakePatternRgbaTexture(4, 4);
  const uint32_t source_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc source_desc;
  source_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &source,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(source_fbo, source_desc));

  cg::rhi::BlitFramebufferDesc blit_desc;
  blit_desc.read_framebuffer = source_fbo;
  blit_desc.draw_framebuffer = 0;
  blit_desc.draw_origin = {-2, -2};
  blit_desc.size = {4, 4};
  blit_desc.draw_size = {4, 4};
  blit_desc.mask = cg::rhi::ClearMask::kColor;
  blit_desc.filter = cg::rhi::FilterMode::kNearest;
  cg::rhi::GetDevice().BlitFramebuffer(blit_desc);

  std::array<uint8_t, 16> readback = {};
  cg::rhi::ReadPixelsDesc readback_desc;
  readback_desc.framebuffer = 0;
  readback_desc.attachment_type = cg::rhi::AttachmentType::kColor;
  readback_desc.read_buffer = cg::rhi::ReadBuffer::kBack;
  readback_desc.origin = {0, 0};
  readback_desc.size = {2, 2};
  readback_desc.format = cg::rhi::PixelFormat::kRGBA;
  readback_desc.type = cg::rhi::PixelType::kUInt8;
  cg::rhi::GetDevice().ReadPixels(readback_desc, readback.data());

  for (int row = 0; row < 2; ++row) {
    for (int column = 0; column < 2; ++column) {
      const std::array<uint8_t, 4> expected = PatternPixel(row + 2, column + 2);
      const size_t offset = static_cast<size_t>(row * 2 + column) * 4;
      EXPECT_EQ(readback[offset + 0], expected[0]);
      EXPECT_EQ(readback[offset + 1], expected[1]);
      EXPECT_EQ(readback[offset + 2], expected[2]);
      EXPECT_EQ(readback[offset + 3], expected[3]);
    }
  }
}

TEST(vulkan_device_test, StandaloneDeviceSupportsLinearScaledAttachmentToDefaultFramebufferBlit) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::Texture source = MakeCornerRgbaFloatTexture();
  const uint32_t source_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc source_desc;
  source_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &source,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(source_fbo, source_desc));

  cg::rhi::BlitFramebufferDesc blit_desc;
  blit_desc.read_framebuffer = source_fbo;
  blit_desc.draw_framebuffer = 0;
  blit_desc.size = {2, 2};
  blit_desc.draw_size = {3, 3};
  blit_desc.mask = cg::rhi::ClearMask::kColor;
  blit_desc.filter = cg::rhi::FilterMode::kLinear;
  cg::rhi::GetDevice().BlitFramebuffer(blit_desc);

  std::array<uint8_t, 36> readback = {};
  cg::rhi::ReadPixelsDesc readback_desc;
  readback_desc.framebuffer = 0;
  readback_desc.attachment_type = cg::rhi::AttachmentType::kColor;
  readback_desc.read_buffer = cg::rhi::ReadBuffer::kBack;
  readback_desc.origin = {0, 0};
  readback_desc.size = {3, 3};
  readback_desc.format = cg::rhi::PixelFormat::kRGBA;
  readback_desc.type = cg::rhi::PixelType::kUInt8;
  cg::rhi::GetDevice().ReadPixels(readback_desc, readback.data());

  const size_t center_offset = static_cast<size_t>(1 * 3 + 1) * 4;
  EXPECT_EQ(readback[center_offset + 0], 128u);
  EXPECT_EQ(readback[center_offset + 1], 128u);
  EXPECT_EQ(readback[center_offset + 2], 0u);
  EXPECT_EQ(readback[center_offset + 3], 255u);

  const size_t top_left_offset = 0;
  EXPECT_EQ(readback[top_left_offset + 0], 0u);
  EXPECT_EQ(readback[top_left_offset + 1], 0u);
  EXPECT_EQ(readback[top_left_offset + 2], 0u);
  EXPECT_EQ(readback[top_left_offset + 3], 255u);

  const size_t bottom_right_offset = static_cast<size_t>(2 * 3 + 2) * 4;
  EXPECT_EQ(readback[bottom_right_offset + 0], 255u);
  EXPECT_EQ(readback[bottom_right_offset + 1], 255u);
  EXPECT_EQ(readback[bottom_right_offset + 2], 0u);
  EXPECT_EQ(readback[bottom_right_offset + 3], 255u);
}

TEST(vulkan_device_test, StandaloneDeviceSupportsScaledDefaultFramebufferSourceBlit) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::Texture source = MakePatternRgbaTexture(2, 2);
  const uint32_t source_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc source_desc;
  source_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &source,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(source_fbo, source_desc));

  cg::rhi::BlitFramebufferDesc to_default_blit_desc;
  to_default_blit_desc.read_framebuffer = source_fbo;
  to_default_blit_desc.draw_framebuffer = 0;
  to_default_blit_desc.size = {2, 2};
  to_default_blit_desc.mask = cg::rhi::ClearMask::kColor;
  to_default_blit_desc.filter = cg::rhi::FilterMode::kNearest;
  cg::rhi::GetDevice().BlitFramebuffer(to_default_blit_desc);

  cg::Texture destination = MakeRgbaTexture(4, 4, {0u, 0u, 0u, 0u});
  const uint32_t destination_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc destination_desc;
  destination_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &destination,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(destination_fbo, destination_desc));

  cg::rhi::BlitFramebufferDesc from_default_blit_desc;
  from_default_blit_desc.read_framebuffer = 0;
  from_default_blit_desc.draw_framebuffer = destination_fbo;
  from_default_blit_desc.read_buffer = cg::rhi::ReadBuffer::kBack;
  from_default_blit_desc.size = {2, 2};
  from_default_blit_desc.draw_size = {4, 4};
  from_default_blit_desc.mask = cg::rhi::ClearMask::kColor;
  from_default_blit_desc.filter = cg::rhi::FilterMode::kNearest;
  cg::rhi::GetDevice().BlitFramebuffer(from_default_blit_desc);

  const std::vector<unsigned char> destination_pixels = destination.GetData<unsigned char>();
  ASSERT_EQ(destination_pixels.size(), 64u);
  for (int row = 0; row < 4; ++row) {
    for (int column = 0; column < 4; ++column) {
      const std::array<uint8_t, 4> expected = PatternPixel(row / 2, column / 2);
      const size_t offset = static_cast<size_t>(row * 4 + column) * 4;
      EXPECT_EQ(destination_pixels[offset + 0], expected[0]);
      EXPECT_EQ(destination_pixels[offset + 1], expected[1]);
      EXPECT_EQ(destination_pixels[offset + 2], expected[2]);
      EXPECT_EQ(destination_pixels[offset + 3], expected[3]);
    }
  }
}

TEST(vulkan_device_test, StandaloneDeviceClipsScaledDefaultFramebufferSourceBlitWhenDestinationRectExceedsBounds) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::Texture source = MakePatternRgbaTexture(4, 4);
  const uint32_t source_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc source_desc;
  source_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &source,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(source_fbo, source_desc));

  cg::rhi::BlitFramebufferDesc to_default_blit_desc;
  to_default_blit_desc.read_framebuffer = source_fbo;
  to_default_blit_desc.draw_framebuffer = 0;
  to_default_blit_desc.size = {4, 4};
  to_default_blit_desc.mask = cg::rhi::ClearMask::kColor;
  to_default_blit_desc.filter = cg::rhi::FilterMode::kNearest;
  cg::rhi::GetDevice().BlitFramebuffer(to_default_blit_desc);

  cg::Texture destination = MakeRgbaTexture(4, 4, {0u, 0u, 0u, 0u});
  const uint32_t destination_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc destination_desc;
  destination_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &destination,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(destination_fbo, destination_desc));

  cg::rhi::BlitFramebufferDesc from_default_blit_desc;
  from_default_blit_desc.read_framebuffer = 0;
  from_default_blit_desc.draw_framebuffer = destination_fbo;
  from_default_blit_desc.read_buffer = cg::rhi::ReadBuffer::kBack;
  from_default_blit_desc.size = {4, 4};
  from_default_blit_desc.draw_size = {8, 8};
  from_default_blit_desc.mask = cg::rhi::ClearMask::kColor;
  from_default_blit_desc.filter = cg::rhi::FilterMode::kNearest;
  cg::rhi::GetDevice().BlitFramebuffer(from_default_blit_desc);

  const std::vector<unsigned char> destination_pixels = destination.GetData<unsigned char>();
  ASSERT_EQ(destination_pixels.size(), 64u);
  for (int row = 0; row < 4; ++row) {
    for (int column = 0; column < 4; ++column) {
      const std::array<uint8_t, 4> expected = PatternPixel(row / 2, column / 2);
      const size_t offset = static_cast<size_t>(row * 4 + column) * 4;
      EXPECT_EQ(destination_pixels[offset + 0], expected[0]);
      EXPECT_EQ(destination_pixels[offset + 1], expected[1]);
      EXPECT_EQ(destination_pixels[offset + 2], expected[2]);
      EXPECT_EQ(destination_pixels[offset + 3], expected[3]);
    }
  }
}

TEST(vulkan_device_test, StandaloneDeviceSupportsNegativeSourceOriginDefaultFramebufferBlit) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::Texture source = MakePatternRgbaTexture(4, 4);
  const uint32_t source_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc source_desc;
  source_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &source,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(source_fbo, source_desc));

  cg::rhi::BlitFramebufferDesc to_default_blit_desc;
  to_default_blit_desc.read_framebuffer = source_fbo;
  to_default_blit_desc.draw_framebuffer = 0;
  to_default_blit_desc.size = {4, 4};
  to_default_blit_desc.mask = cg::rhi::ClearMask::kColor;
  to_default_blit_desc.filter = cg::rhi::FilterMode::kNearest;
  cg::rhi::GetDevice().BlitFramebuffer(to_default_blit_desc);

  cg::Texture destination = MakeRgbaTexture(4, 4, {0u, 0u, 0u, 0u});
  const uint32_t destination_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc destination_desc;
  destination_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &destination,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(destination_fbo, destination_desc));

  cg::rhi::BlitFramebufferDesc from_default_blit_desc;
  from_default_blit_desc.read_framebuffer = 0;
  from_default_blit_desc.draw_framebuffer = destination_fbo;
  from_default_blit_desc.read_buffer = cg::rhi::ReadBuffer::kBack;
  from_default_blit_desc.read_origin = {-2, -2};
  from_default_blit_desc.size = {4, 4};
  from_default_blit_desc.draw_size = {4, 4};
  from_default_blit_desc.mask = cg::rhi::ClearMask::kColor;
  from_default_blit_desc.filter = cg::rhi::FilterMode::kNearest;
  cg::rhi::GetDevice().BlitFramebuffer(from_default_blit_desc);

  const std::vector<unsigned char> destination_pixels = destination.GetData<unsigned char>();
  ASSERT_EQ(destination_pixels.size(), 64u);
  for (int row = 0; row < 4; ++row) {
    for (int column = 0; column < 4; ++column) {
      const size_t offset = static_cast<size_t>(row * 4 + column) * 4;
      if (row >= 2 && column >= 2) {
        const std::array<uint8_t, 4> expected = PatternPixel(row - 2, column - 2);
        EXPECT_EQ(destination_pixels[offset + 0], expected[0]);
        EXPECT_EQ(destination_pixels[offset + 1], expected[1]);
        EXPECT_EQ(destination_pixels[offset + 2], expected[2]);
        EXPECT_EQ(destination_pixels[offset + 3], expected[3]);
      } else {
        EXPECT_EQ(destination_pixels[offset + 0], 0u);
        EXPECT_EQ(destination_pixels[offset + 1], 0u);
        EXPECT_EQ(destination_pixels[offset + 2], 0u);
        EXPECT_EQ(destination_pixels[offset + 3], 0u);
      }
    }
  }
}

TEST(vulkan_device_test, StandaloneDeviceSupportsLinearScaledDefaultFramebufferSourceBlit) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::Texture source = MakeCornerRgbaTexture();
  const uint32_t source_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc source_desc;
  source_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &source,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(source_fbo, source_desc));

  cg::rhi::BlitFramebufferDesc to_default_blit_desc;
  to_default_blit_desc.read_framebuffer = source_fbo;
  to_default_blit_desc.draw_framebuffer = 0;
  to_default_blit_desc.size = {2, 2};
  to_default_blit_desc.mask = cg::rhi::ClearMask::kColor;
  to_default_blit_desc.filter = cg::rhi::FilterMode::kNearest;
  cg::rhi::GetDevice().BlitFramebuffer(to_default_blit_desc);

  cg::Texture destination = MakeRgbaTexture(3, 3, {0u, 0u, 0u, 0u});
  const uint32_t destination_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc destination_desc;
  destination_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &destination,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(destination_fbo, destination_desc));

  cg::rhi::BlitFramebufferDesc from_default_blit_desc;
  from_default_blit_desc.read_framebuffer = 0;
  from_default_blit_desc.draw_framebuffer = destination_fbo;
  from_default_blit_desc.read_buffer = cg::rhi::ReadBuffer::kBack;
  from_default_blit_desc.size = {2, 2};
  from_default_blit_desc.draw_size = {3, 3};
  from_default_blit_desc.mask = cg::rhi::ClearMask::kColor;
  from_default_blit_desc.filter = cg::rhi::FilterMode::kLinear;
  cg::rhi::GetDevice().BlitFramebuffer(from_default_blit_desc);

  const std::vector<unsigned char> destination_pixels = destination.GetData<unsigned char>();
  ASSERT_EQ(destination_pixels.size(), 36u);

  const size_t center_offset = static_cast<size_t>(1 * 3 + 1) * 4;
  EXPECT_EQ(destination_pixels[center_offset + 0], 128u);
  EXPECT_EQ(destination_pixels[center_offset + 1], 128u);
  EXPECT_EQ(destination_pixels[center_offset + 2], 0u);
  EXPECT_EQ(destination_pixels[center_offset + 3], 255u);

  const size_t top_left_offset = 0;
  EXPECT_EQ(destination_pixels[top_left_offset + 0], 0u);
  EXPECT_EQ(destination_pixels[top_left_offset + 1], 0u);
  EXPECT_EQ(destination_pixels[top_left_offset + 2], 0u);
  EXPECT_EQ(destination_pixels[top_left_offset + 3], 255u);

  const size_t bottom_right_offset = static_cast<size_t>(2 * 3 + 2) * 4;
  EXPECT_EQ(destination_pixels[bottom_right_offset + 0], 255u);
  EXPECT_EQ(destination_pixels[bottom_right_offset + 1], 255u);
  EXPECT_EQ(destination_pixels[bottom_right_offset + 2], 0u);
  EXPECT_EQ(destination_pixels[bottom_right_offset + 3], 255u);
}

TEST(vulkan_device_test, StandaloneDeviceSupportsOffsetDefaultFramebufferBlit) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::Texture source = MakePatternRgbaTexture(4, 4);
  const uint32_t source_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc source_desc;
  source_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &source,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(source_fbo, source_desc));

  cg::rhi::RenderPassDesc default_pass_desc;
  default_pass_desc.framebuffer = 0;
  default_pass_desc.viewport_size = {4, 4};
  default_pass_desc.color_attachments.push_back({
      .attachment_index = 0,
      .clear = true,
      .clear_color = {0.0f, 0.0f, 0.0f, 0.0f},
  });
  cg::rhi::GetDevice().BeginRenderPass(default_pass_desc);
  cg::rhi::GetDevice().EndRenderPass();

  cg::rhi::BlitFramebufferDesc to_default_blit_desc;
  to_default_blit_desc.read_framebuffer = source_fbo;
  to_default_blit_desc.draw_framebuffer = 0;
  to_default_blit_desc.read_origin = {1, 1};
  to_default_blit_desc.draw_origin = {0, 0};
  to_default_blit_desc.size = {2, 2};
  to_default_blit_desc.mask = cg::rhi::ClearMask::kColor;
  to_default_blit_desc.filter = cg::rhi::FilterMode::kNearest;
  cg::rhi::GetDevice().BlitFramebuffer(to_default_blit_desc);

  std::array<uint8_t, 64> default_readback = {};
  cg::rhi::ReadPixelsDesc default_readback_desc;
  default_readback_desc.framebuffer = 0;
  default_readback_desc.attachment_type = cg::rhi::AttachmentType::kColor;
  default_readback_desc.read_buffer = cg::rhi::ReadBuffer::kBack;
  default_readback_desc.origin = {0, 0};
  default_readback_desc.size = {4, 4};
  default_readback_desc.format = cg::rhi::PixelFormat::kRGBA;
  default_readback_desc.type = cg::rhi::PixelType::kUInt8;
  cg::rhi::GetDevice().ReadPixels(default_readback_desc, default_readback.data());
  for (int row = 0; row < 4; ++row) {
    for (int column = 0; column < 4; ++column) {
      const size_t offset = static_cast<size_t>(row * 4 + column) * 4;
      if (row < 2 && column < 2) {
        const std::array<uint8_t, 4> expected = PatternPixel(row + 1, column + 1);
        EXPECT_EQ(default_readback[offset + 0], expected[0]);
        EXPECT_EQ(default_readback[offset + 1], expected[1]);
        EXPECT_EQ(default_readback[offset + 2], expected[2]);
        EXPECT_EQ(default_readback[offset + 3], expected[3]);
      } else {
        EXPECT_EQ(default_readback[offset + 0], 0u);
        EXPECT_EQ(default_readback[offset + 1], 0u);
        EXPECT_EQ(default_readback[offset + 2], 0u);
        EXPECT_EQ(default_readback[offset + 3], 0u);
      }
    }
  }

  cg::Texture destination = MakeRgbaTexture(4, 4, {0u, 0u, 0u, 0u});
  const uint32_t destination_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc destination_desc;
  destination_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &destination,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(destination_fbo, destination_desc));

  cg::rhi::BlitFramebufferDesc from_default_blit_desc;
  from_default_blit_desc.read_framebuffer = 0;
  from_default_blit_desc.draw_framebuffer = destination_fbo;
  from_default_blit_desc.read_origin = {0, 0};
  from_default_blit_desc.draw_origin = {2, 1};
  from_default_blit_desc.size = {2, 2};
  from_default_blit_desc.mask = cg::rhi::ClearMask::kColor;
  from_default_blit_desc.filter = cg::rhi::FilterMode::kNearest;
  cg::rhi::GetDevice().BlitFramebuffer(from_default_blit_desc);

  const std::vector<unsigned char> destination_pixels = destination.GetData<unsigned char>();
  ASSERT_EQ(destination_pixels.size(), 64u);
  for (int row = 0; row < 4; ++row) {
    for (int column = 0; column < 4; ++column) {
      const size_t offset = static_cast<size_t>(row * 4 + column) * 4;
      if (row >= 1 && row < 3 && column >= 2 && column < 4) {
        const std::array<uint8_t, 4> expected = PatternPixel(row, column - 1);
        EXPECT_EQ(destination_pixels[offset + 0], expected[0]);
        EXPECT_EQ(destination_pixels[offset + 1], expected[1]);
        EXPECT_EQ(destination_pixels[offset + 2], expected[2]);
        EXPECT_EQ(destination_pixels[offset + 3], expected[3]);
      } else {
        EXPECT_EQ(destination_pixels[offset + 0], 0u);
        EXPECT_EQ(destination_pixels[offset + 1], 0u);
        EXPECT_EQ(destination_pixels[offset + 2], 0u);
        EXPECT_EQ(destination_pixels[offset + 3], 0u);
      }
    }
  }
}

TEST(vulkan_device_test, StandaloneDeviceRejectsMismatchedFramebufferAttachmentSizes) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::Texture color = MakeRgbaTexture(2, 2, {0u, 0u, 0u, 0u});
  cg::Texture depth = MakeDepthTexture(4, 4, 1.0f);

  const uint32_t framebuffer = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc framebuffer_desc;
  framebuffer_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &color,
  });
  framebuffer_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kDepth,
      .attachment_index = 0,
      .texture = &depth,
  });

  EXPECT_FALSE(cg::rhi::GetDevice().ConfigureFramebuffer(framebuffer, framebuffer_desc));
}

TEST(vulkan_device_test, StandaloneDeviceRejectsRenderPassOnIncompleteFramebuffer) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  EXPECT_DEATH(
      ([] {
        auto device = cg::rhi::CreateVulkanDevice(false);
        if (device == nullptr) {
          std::abort();
        }
        cg::rhi::SetDevice(std::move(device));
        cg::Texture color = MakeRgbaTexture(2, 2, {0u, 0u, 0u, 0u});
        cg::Texture depth = MakeDepthTexture(4, 4, 1.0f);

        const uint32_t framebuffer = cg::rhi::GetDevice().CreateFramebuffer();
        cg::rhi::FramebufferDesc framebuffer_desc;
        framebuffer_desc.attachments.push_back({
            .attachment_type = cg::rhi::AttachmentType::kColor,
            .attachment_index = 0,
            .texture = &color,
        });
        framebuffer_desc.attachments.push_back({
            .attachment_type = cg::rhi::AttachmentType::kDepth,
            .attachment_index = 0,
            .texture = &depth,
        });
        if (cg::rhi::GetDevice().ConfigureFramebuffer(framebuffer, framebuffer_desc)) {
          std::abort();
        }

        cg::rhi::RenderPassDesc render_pass_desc;
        render_pass_desc.framebuffer = framebuffer;
        render_pass_desc.viewport_size = {2, 2};
        cg::rhi::GetDevice().BeginRenderPass(render_pass_desc);
      }()),
      ".*");
}

TEST(vulkan_device_test, StandaloneDeviceRejectsDefaultFramebufferRenderPassWithUnsupportedColorAttachmentIndex) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  EXPECT_DEATH(
      ([] {
        auto device = cg::rhi::CreateVulkanDevice(false);
        if (device == nullptr) {
          std::abort();
        }
        cg::rhi::SetDevice(std::move(device));

        cg::rhi::RenderPassDesc render_pass_desc;
        render_pass_desc.framebuffer = 0;
        render_pass_desc.viewport_size = {1, 1};
        render_pass_desc.color_attachments.push_back({
            .attachment_index = 1,
            .clear = false,
        });
        cg::rhi::GetDevice().BeginRenderPass(render_pass_desc);
      }()),
      ".*");
}

TEST(vulkan_device_test, StandaloneDeviceRejectsRenderPassOnMissingNonDefaultColorAttachment) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  EXPECT_DEATH(
      ([] {
        auto device = cg::rhi::CreateVulkanDevice(false);
        if (device == nullptr) {
          std::abort();
        }
        cg::rhi::SetDevice(std::move(device));

        cg::Texture color0 = MakeRgbaTexture(2, 2, {0u, 0u, 0u, 0u});
        const uint32_t framebuffer = cg::rhi::GetDevice().CreateFramebuffer();
        cg::rhi::FramebufferDesc framebuffer_desc;
        framebuffer_desc.attachments.push_back({
            .attachment_type = cg::rhi::AttachmentType::kColor,
            .attachment_index = 0,
            .texture = &color0,
        });
        if (!cg::rhi::GetDevice().ConfigureFramebuffer(framebuffer, framebuffer_desc)) {
          std::abort();
        }

        cg::rhi::RenderPassDesc render_pass_desc;
        render_pass_desc.framebuffer = framebuffer;
        render_pass_desc.viewport_size = {2, 2};
        render_pass_desc.color_attachments.push_back({
            .attachment_index = 1,
            .clear = true,
            .clear_color = {1.0f, 0.0f, 0.0f, 1.0f},
        });
        cg::rhi::GetDevice().BeginRenderPass(render_pass_desc);
      }()),
      ".*");
}

TEST(vulkan_device_test, StandaloneDeviceRejectsRenderPassDepthClearWithoutDepthAttachment) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  EXPECT_DEATH(
      ([] {
        auto device = cg::rhi::CreateVulkanDevice(false);
        if (device == nullptr) {
          std::abort();
        }
        cg::rhi::SetDevice(std::move(device));

        cg::Texture color0 = MakeRgbaTexture(2, 2, {0u, 0u, 0u, 0u});
        const uint32_t framebuffer = cg::rhi::GetDevice().CreateFramebuffer();
        cg::rhi::FramebufferDesc framebuffer_desc;
        framebuffer_desc.attachments.push_back({
            .attachment_type = cg::rhi::AttachmentType::kColor,
            .attachment_index = 0,
            .texture = &color0,
        });
        if (!cg::rhi::GetDevice().ConfigureFramebuffer(framebuffer, framebuffer_desc)) {
          std::abort();
        }

        cg::rhi::RenderPassDesc render_pass_desc;
        render_pass_desc.framebuffer = framebuffer;
        render_pass_desc.viewport_size = {2, 2};
        render_pass_desc.clear_depth = true;
        render_pass_desc.depth_clear_value = 0.5f;
        cg::rhi::GetDevice().BeginRenderPass(render_pass_desc);
      }()),
      ".*");
}

TEST(vulkan_device_test, StandaloneDeviceRejectsRenderPassStencilClearWithoutStencilAttachment) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  EXPECT_DEATH(
      ([] {
        auto device = cg::rhi::CreateVulkanDevice(false);
        if (device == nullptr) {
          std::abort();
        }
        cg::rhi::SetDevice(std::move(device));

        cg::Texture color0 = MakeRgbaTexture(2, 2, {0u, 0u, 0u, 0u});
        const uint32_t framebuffer = cg::rhi::GetDevice().CreateFramebuffer();
        cg::rhi::FramebufferDesc framebuffer_desc;
        framebuffer_desc.attachments.push_back({
            .attachment_type = cg::rhi::AttachmentType::kColor,
            .attachment_index = 0,
            .texture = &color0,
        });
        if (!cg::rhi::GetDevice().ConfigureFramebuffer(framebuffer, framebuffer_desc)) {
          std::abort();
        }

        cg::rhi::RenderPassDesc render_pass_desc;
        render_pass_desc.framebuffer = framebuffer;
        render_pass_desc.viewport_size = {2, 2};
        render_pass_desc.clear_stencil = true;
        render_pass_desc.stencil_clear_value = 5;
        cg::rhi::GetDevice().BeginRenderPass(render_pass_desc);
      }()),
      ".*");
}

TEST(vulkan_device_test, StandaloneDeviceRejectsRenderPassWithZeroViewportSize) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  EXPECT_DEATH(
      ([] {
        auto device = cg::rhi::CreateVulkanDevice(false);
        if (device == nullptr) {
          std::abort();
        }
        cg::rhi::SetDevice(std::move(device));

        cg::rhi::RenderPassDesc render_pass_desc;
        render_pass_desc.framebuffer = 0;
        render_pass_desc.viewport_size = {0, 1};
        cg::rhi::GetDevice().BeginRenderPass(render_pass_desc);
      }()),
      ".*");
}

TEST(vulkan_device_test, StandaloneDeviceRejectsRenderPassWithNegativeViewportSize) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  EXPECT_DEATH(
      ([] {
        auto device = cg::rhi::CreateVulkanDevice(false);
        if (device == nullptr) {
          std::abort();
        }
        cg::rhi::SetDevice(std::move(device));

        cg::rhi::RenderPassDesc render_pass_desc;
        render_pass_desc.framebuffer = 0;
        render_pass_desc.viewport_size = {1, -1};
        cg::rhi::GetDevice().BeginRenderPass(render_pass_desc);
      }()),
      ".*");
}

TEST(vulkan_device_test, StandaloneDeviceSupportsAttachmentIndexedFramebufferBlit) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::Texture source_color0 = MakeRgbaTexture(2, 2, {0u, 0u, 0u, 0u});
  cg::Texture source_color1 = MakeRgbaTexture(2, 2, {0u, 0u, 0u, 0u});
  cg::Texture destination_color0 = MakeRgbaTexture(2, 2, {0u, 0u, 0u, 0u});
  cg::Texture destination_color1 = MakeRgbaTexture(2, 2, {0u, 0u, 0u, 0u});

  const uint32_t source_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc source_desc;
  source_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &source_color0,
  });
  source_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 1,
      .texture = &source_color1,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(source_fbo, source_desc));

  cg::rhi::RenderPassDesc source_pass_desc;
  source_pass_desc.framebuffer = source_fbo;
  source_pass_desc.viewport_size = {2, 2};
  source_pass_desc.color_attachments.push_back({
      .attachment_index = 1,
      .clear = true,
      .clear_color = {0.0f, 0.0f, 1.0f, 1.0f},
  });
  cg::rhi::GetDevice().BeginRenderPass(source_pass_desc);
  cg::rhi::GetDevice().EndRenderPass();

  const uint32_t destination_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc destination_desc;
  destination_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &destination_color0,
  });
  destination_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 1,
      .texture = &destination_color1,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(destination_fbo, destination_desc));

  cg::rhi::BlitFramebufferDesc blit_desc;
  blit_desc.read_framebuffer = source_fbo;
  blit_desc.draw_framebuffer = destination_fbo;
  blit_desc.read_color_attachment_index = 1;
  blit_desc.draw_color_attachment_index = 1;
  blit_desc.size = {2, 2};
  blit_desc.mask = cg::rhi::ClearMask::kColor;
  blit_desc.filter = cg::rhi::FilterMode::kNearest;
  cg::rhi::GetDevice().BlitFramebuffer(blit_desc);

  cg::rhi::ReadPixelsDesc readback_desc;
  readback_desc.framebuffer = destination_fbo;
  readback_desc.attachment_type = cg::rhi::AttachmentType::kColor;
  readback_desc.attachment_index = 1;
  readback_desc.origin = {0, 0};
  readback_desc.size = {2, 2};
  readback_desc.format = cg::rhi::PixelFormat::kRGBA;
  readback_desc.type = cg::rhi::PixelType::kUInt8;
  std::array<uint8_t, 16> readback = {};
  cg::rhi::GetDevice().ReadPixels(readback_desc, readback.data());
  for (size_t i = 0; i < readback.size(); i += 4) {
    EXPECT_EQ(readback[i + 0], 0u);
    EXPECT_EQ(readback[i + 1], 0u);
    EXPECT_EQ(readback[i + 2], 255u);
    EXPECT_EQ(readback[i + 3], 255u);
  }

  const std::vector<unsigned char> untouched_pixels = destination_color0.GetData<unsigned char>();
  EXPECT_TRUE(std::all_of(untouched_pixels.begin(), untouched_pixels.end(), [](unsigned char value) {
    return value == 0u;
  }));
}

TEST(vulkan_device_test, StandaloneDeviceSupportsCrossFormatColorAttachmentBlit) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::Texture source = MakeRgbaTexture(2, 2, {0u, 128u, 255u, 255u});
  cg::Texture destination = MakeRgbaFloatTexture(2, 2, {0.0f, 0.0f, 0.0f, 0.0f});

  const uint32_t source_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc source_desc;
  source_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &source,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(source_fbo, source_desc));

  const uint32_t destination_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc destination_desc;
  destination_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &destination,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(destination_fbo, destination_desc));

  cg::rhi::BlitFramebufferDesc blit_desc;
  blit_desc.read_framebuffer = source_fbo;
  blit_desc.draw_framebuffer = destination_fbo;
  blit_desc.size = {2, 2};
  blit_desc.mask = cg::rhi::ClearMask::kColor;
  blit_desc.filter = cg::rhi::FilterMode::kNearest;
  cg::rhi::GetDevice().BlitFramebuffer(blit_desc);

  std::array<float, 16> readback = {};
  cg::rhi::ReadPixelsDesc readback_desc;
  readback_desc.framebuffer = destination_fbo;
  readback_desc.attachment_type = cg::rhi::AttachmentType::kColor;
  readback_desc.attachment_index = 0;
  readback_desc.origin = {0, 0};
  readback_desc.size = {2, 2};
  readback_desc.format = cg::rhi::PixelFormat::kRGBA;
  readback_desc.type = cg::rhi::PixelType::kFloat32;
  cg::rhi::GetDevice().ReadPixels(readback_desc, readback.data());
  for (size_t i = 0; i < readback.size(); i += 4) {
    EXPECT_FLOAT_EQ(readback[i + 0], 0.0f);
    EXPECT_NEAR(readback[i + 1], 128.0f / 255.0f, 1e-6f);
    EXPECT_FLOAT_EQ(readback[i + 2], 1.0f);
    EXPECT_FLOAT_EQ(readback[i + 3], 1.0f);
  }

  const std::vector<float> destination_pixels = destination.GetData<float>();
  ASSERT_EQ(destination_pixels.size(), 16u);
  for (size_t i = 0; i < destination_pixels.size(); i += 4) {
    EXPECT_FLOAT_EQ(destination_pixels[i + 0], 0.0f);
    EXPECT_NEAR(destination_pixels[i + 1], 128.0f / 255.0f, 1e-6f);
    EXPECT_FLOAT_EQ(destination_pixels[i + 2], 1.0f);
    EXPECT_FLOAT_EQ(destination_pixels[i + 3], 1.0f);
  }
}

TEST(vulkan_device_test, StandaloneDeviceSupportsAttachmentColorReadbackConversion) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::Texture color = MakeRgbaFloatTexture(2, 2, {0.0f, 0.0f, 0.0f, 0.0f});
  const uint32_t framebuffer = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc framebuffer_desc;
  framebuffer_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &color,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(framebuffer, framebuffer_desc));

  cg::rhi::RenderPassDesc render_pass_desc;
  render_pass_desc.framebuffer = framebuffer;
  render_pass_desc.viewport_size = {2, 2};
  render_pass_desc.color_attachments.push_back({
      .attachment_index = 0,
      .clear = true,
      .clear_color = {0.25f, 0.5f, 1.0f, 1.0f},
  });
  cg::rhi::GetDevice().BeginRenderPass(render_pass_desc);
  cg::rhi::GetDevice().EndRenderPass();

  std::array<uint8_t, 16> readback = {};
  cg::rhi::ReadPixelsDesc readback_desc;
  readback_desc.framebuffer = framebuffer;
  readback_desc.attachment_type = cg::rhi::AttachmentType::kColor;
  readback_desc.attachment_index = 0;
  readback_desc.origin = {0, 0};
  readback_desc.size = {2, 2};
  readback_desc.format = cg::rhi::PixelFormat::kRGBA;
  readback_desc.type = cg::rhi::PixelType::kUInt8;
  cg::rhi::GetDevice().ReadPixels(readback_desc, readback.data());

  for (size_t i = 0; i < readback.size(); i += 4) {
    EXPECT_EQ(readback[i + 0], 64u);
    EXPECT_EQ(readback[i + 1], 128u);
    EXPECT_EQ(readback[i + 2], 255u);
    EXPECT_EQ(readback[i + 3], 255u);
  }
}

TEST(vulkan_device_test, StandaloneDeviceSupportsCustomDepthClearValue) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::Texture depth = MakeDepthTexture(2, 2, 1.0f);
  const uint32_t framebuffer = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc framebuffer_desc;
  framebuffer_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kDepth,
      .attachment_index = 0,
      .texture = &depth,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(framebuffer, framebuffer_desc));

  cg::rhi::RenderPassDesc render_pass_desc;
  render_pass_desc.framebuffer = framebuffer;
  render_pass_desc.viewport_size = {2, 2};
  render_pass_desc.clear_depth = true;
  render_pass_desc.depth_clear_value = 0.25f;
  cg::rhi::GetDevice().BeginRenderPass(render_pass_desc);
  cg::rhi::GetDevice().EndRenderPass();

  const std::vector<float> depth_values = depth.GetData<float>();
  ASSERT_EQ(depth_values.size(), 4u);
  for (float value : depth_values) {
    EXPECT_FLOAT_EQ(value, 0.25f);
  }
}

TEST(vulkan_device_test, StandaloneDeviceSupportsCustomStencilClearValue) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::Texture stencil = MakeStencilTexture(2, 2, 0u);
  const uint32_t framebuffer = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc framebuffer_desc;
  framebuffer_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kStencil,
      .attachment_index = 0,
      .texture = &stencil,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(framebuffer, framebuffer_desc));

  cg::rhi::RenderPassDesc render_pass_desc;
  render_pass_desc.framebuffer = framebuffer;
  render_pass_desc.viewport_size = {2, 2};
  render_pass_desc.clear_stencil = true;
  render_pass_desc.stencil_clear_value = 7;
  cg::rhi::GetDevice().BeginRenderPass(render_pass_desc);
  cg::rhi::GetDevice().EndRenderPass();

  const std::vector<uint8_t> stencil_values = stencil.GetData<uint8_t>();
  ASSERT_EQ(stencil_values.size(), 4u);
  for (uint8_t value : stencil_values) {
    EXPECT_EQ(value, 7u);
  }
}

TEST(vulkan_device_test, StandaloneDeviceSupportsDepthAndStencilAttachmentReadback) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::Texture depth = MakeDepthTexture(2, 2, 1.0f);
  cg::Texture stencil = MakeStencilTexture(2, 2, 0u);
  const uint32_t framebuffer = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc framebuffer_desc;
  framebuffer_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kDepth,
      .attachment_index = 0,
      .texture = &depth,
  });
  framebuffer_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kStencil,
      .attachment_index = 0,
      .texture = &stencil,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(framebuffer, framebuffer_desc));

  cg::rhi::RenderPassDesc render_pass_desc;
  render_pass_desc.framebuffer = framebuffer;
  render_pass_desc.viewport_size = {2, 2};
  render_pass_desc.clear_depth = true;
  render_pass_desc.depth_clear_value = 0.25f;
  render_pass_desc.clear_stencil = true;
  render_pass_desc.stencil_clear_value = 9;
  cg::rhi::GetDevice().BeginRenderPass(render_pass_desc);
  cg::rhi::GetDevice().EndRenderPass();

  std::array<float, 4> depth_readback = {};
  cg::rhi::ReadPixelsDesc depth_readback_desc;
  depth_readback_desc.framebuffer = framebuffer;
  depth_readback_desc.attachment_type = cg::rhi::AttachmentType::kDepth;
  depth_readback_desc.attachment_index = 0;
  depth_readback_desc.origin = {0, 0};
  depth_readback_desc.size = {2, 2};
  depth_readback_desc.format = cg::rhi::PixelFormat::kDepthComponent;
  depth_readback_desc.type = cg::rhi::PixelType::kFloat32;
  cg::rhi::GetDevice().ReadPixels(depth_readback_desc, depth_readback.data());
  for (float value : depth_readback) {
    EXPECT_FLOAT_EQ(value, 0.25f);
  }

  std::array<uint8_t, 4> stencil_readback = {};
  cg::rhi::ReadPixelsDesc stencil_readback_desc;
  stencil_readback_desc.framebuffer = framebuffer;
  stencil_readback_desc.attachment_type = cg::rhi::AttachmentType::kStencil;
  stencil_readback_desc.attachment_index = 0;
  stencil_readback_desc.origin = {0, 0};
  stencil_readback_desc.size = {2, 2};
  stencil_readback_desc.format = cg::rhi::PixelFormat::kStencilIndex;
  stencil_readback_desc.type = cg::rhi::PixelType::kUInt8;
  cg::rhi::GetDevice().ReadPixels(stencil_readback_desc, stencil_readback.data());
  for (uint8_t value : stencil_readback) {
    EXPECT_EQ(value, 9u);
  }
}

TEST(vulkan_device_test, StandaloneDeviceSupportsDefaultFramebufferDepthAndStencilBlit) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::Texture source_depth = MakeDepthTexture(2, 2, 1.0f);
  cg::Texture source_stencil = MakeStencilTexture(2, 2, 0u);
  const uint32_t source_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc source_desc;
  source_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kDepth,
      .attachment_index = 0,
      .texture = &source_depth,
  });
  source_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kStencil,
      .attachment_index = 0,
      .texture = &source_stencil,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(source_fbo, source_desc));

  cg::rhi::RenderPassDesc source_pass_desc;
  source_pass_desc.framebuffer = source_fbo;
  source_pass_desc.viewport_size = {2, 2};
  source_pass_desc.clear_depth = true;
  source_pass_desc.depth_clear_value = 0.4f;
  source_pass_desc.clear_stencil = true;
  source_pass_desc.stencil_clear_value = 13;
  cg::rhi::GetDevice().BeginRenderPass(source_pass_desc);
  cg::rhi::GetDevice().EndRenderPass();

  cg::rhi::BlitFramebufferDesc to_default_blit_desc;
  to_default_blit_desc.read_framebuffer = source_fbo;
  to_default_blit_desc.draw_framebuffer = 0;
  to_default_blit_desc.size = {2, 2};
  to_default_blit_desc.mask = cg::rhi::ClearMask::kDepth | cg::rhi::ClearMask::kStencil;
  to_default_blit_desc.filter = cg::rhi::FilterMode::kNearest;
  cg::rhi::GetDevice().BlitFramebuffer(to_default_blit_desc);

  std::array<float, 4> default_depth_readback = {};
  cg::rhi::ReadPixelsDesc default_depth_readback_desc;
  default_depth_readback_desc.framebuffer = 0;
  default_depth_readback_desc.attachment_type = cg::rhi::AttachmentType::kDepth;
  default_depth_readback_desc.read_buffer = cg::rhi::ReadBuffer::kBack;
  default_depth_readback_desc.origin = {0, 0};
  default_depth_readback_desc.size = {2, 2};
  default_depth_readback_desc.format = cg::rhi::PixelFormat::kDepthComponent;
  default_depth_readback_desc.type = cg::rhi::PixelType::kFloat32;
  cg::rhi::GetDevice().ReadPixels(default_depth_readback_desc, default_depth_readback.data());
  for (float value : default_depth_readback) {
    EXPECT_FLOAT_EQ(value, 0.4f);
  }

  std::array<uint8_t, 4> default_stencil_readback = {};
  cg::rhi::ReadPixelsDesc default_stencil_readback_desc;
  default_stencil_readback_desc.framebuffer = 0;
  default_stencil_readback_desc.attachment_type = cg::rhi::AttachmentType::kStencil;
  default_stencil_readback_desc.read_buffer = cg::rhi::ReadBuffer::kBack;
  default_stencil_readback_desc.origin = {0, 0};
  default_stencil_readback_desc.size = {2, 2};
  default_stencil_readback_desc.format = cg::rhi::PixelFormat::kStencilIndex;
  default_stencil_readback_desc.type = cg::rhi::PixelType::kUInt8;
  cg::rhi::GetDevice().ReadPixels(default_stencil_readback_desc, default_stencil_readback.data());
  for (uint8_t value : default_stencil_readback) {
    EXPECT_EQ(value, 13u);
  }

  cg::Texture destination_depth = MakeDepthTexture(2, 2, 1.0f);
  cg::Texture destination_stencil = MakeStencilTexture(2, 2, 0u);
  const uint32_t destination_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc destination_desc;
  destination_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kDepth,
      .attachment_index = 0,
      .texture = &destination_depth,
  });
  destination_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kStencil,
      .attachment_index = 0,
      .texture = &destination_stencil,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(destination_fbo, destination_desc));

  cg::rhi::BlitFramebufferDesc to_destination_blit_desc;
  to_destination_blit_desc.read_framebuffer = 0;
  to_destination_blit_desc.draw_framebuffer = destination_fbo;
  to_destination_blit_desc.size = {2, 2};
  to_destination_blit_desc.mask = cg::rhi::ClearMask::kDepth | cg::rhi::ClearMask::kStencil;
  to_destination_blit_desc.filter = cg::rhi::FilterMode::kNearest;
  cg::rhi::GetDevice().BlitFramebuffer(to_destination_blit_desc);

  const std::vector<float> destination_depth_values = destination_depth.GetData<float>();
  ASSERT_EQ(destination_depth_values.size(), 4u);
  for (float value : destination_depth_values) {
    EXPECT_FLOAT_EQ(value, 0.4f);
  }

  const std::vector<uint8_t> destination_stencil_values = destination_stencil.GetData<uint8_t>();
  ASSERT_EQ(destination_stencil_values.size(), 4u);
  for (uint8_t value : destination_stencil_values) {
    EXPECT_EQ(value, 13u);
  }
}

TEST(vulkan_device_test, StandaloneDeviceSupportsOverlappingDefaultFramebufferDepthAndStencilSelfBlit) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::Texture source_depth = MakePatternDepthTexture(4, 4);
  cg::Texture source_stencil = MakePatternStencilTexture(4, 4);
  const uint32_t source_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc source_desc;
  source_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kDepth,
      .attachment_index = 0,
      .texture = &source_depth,
  });
  source_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kStencil,
      .attachment_index = 0,
      .texture = &source_stencil,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(source_fbo, source_desc));

  cg::rhi::BlitFramebufferDesc to_default_blit_desc;
  to_default_blit_desc.read_framebuffer = source_fbo;
  to_default_blit_desc.draw_framebuffer = 0;
  to_default_blit_desc.size = {4, 4};
  to_default_blit_desc.mask = cg::rhi::ClearMask::kDepth | cg::rhi::ClearMask::kStencil;
  cg::rhi::GetDevice().BlitFramebuffer(to_default_blit_desc);

  cg::rhi::BlitFramebufferDesc self_blit_desc;
  self_blit_desc.read_framebuffer = 0;
  self_blit_desc.draw_framebuffer = 0;
  self_blit_desc.read_origin = {0, 0};
  self_blit_desc.draw_origin = {1, 1};
  self_blit_desc.size = {2, 2};
  self_blit_desc.mask = cg::rhi::ClearMask::kDepth | cg::rhi::ClearMask::kStencil;
  cg::rhi::GetDevice().BlitFramebuffer(self_blit_desc);

  std::array<float, 16> depth_readback = {};
  cg::rhi::ReadPixelsDesc depth_readback_desc;
  depth_readback_desc.framebuffer = 0;
  depth_readback_desc.attachment_type = cg::rhi::AttachmentType::kDepth;
  depth_readback_desc.read_buffer = cg::rhi::ReadBuffer::kBack;
  depth_readback_desc.origin = {0, 0};
  depth_readback_desc.size = {4, 4};
  depth_readback_desc.format = cg::rhi::PixelFormat::kDepthComponent;
  depth_readback_desc.type = cg::rhi::PixelType::kFloat32;
  cg::rhi::GetDevice().ReadPixels(depth_readback_desc, depth_readback.data());

  std::array<uint8_t, 16> stencil_readback = {};
  cg::rhi::ReadPixelsDesc stencil_readback_desc;
  stencil_readback_desc.framebuffer = 0;
  stencil_readback_desc.attachment_type = cg::rhi::AttachmentType::kStencil;
  stencil_readback_desc.read_buffer = cg::rhi::ReadBuffer::kBack;
  stencil_readback_desc.origin = {0, 0};
  stencil_readback_desc.size = {4, 4};
  stencil_readback_desc.format = cg::rhi::PixelFormat::kStencilIndex;
  stencil_readback_desc.type = cg::rhi::PixelType::kUInt8;
  cg::rhi::GetDevice().ReadPixels(stencil_readback_desc, stencil_readback.data());

  for (int row = 0; row < 4; ++row) {
    for (int column = 0; column < 4; ++column) {
      const int expected_source_row = (row >= 1 && row < 3 && column >= 1 && column < 3) ? row - 1 : row;
      const int expected_source_column = (row >= 1 && row < 3 && column >= 1 && column < 3) ? column - 1 : column;
      EXPECT_FLOAT_EQ(depth_readback[static_cast<size_t>(row * 4 + column)],
                      PatternDepthValue(expected_source_row, expected_source_column));
      EXPECT_EQ(stencil_readback[static_cast<size_t>(row * 4 + column)],
                PatternStencilValue(expected_source_row, expected_source_column));
    }
  }
}

TEST(vulkan_device_test, StandaloneDeviceRespectsBlitSizeWhenCopyingAttachmentsToDefaultFramebuffer) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::rhi::RenderPassDesc default_pass_desc;
  default_pass_desc.framebuffer = 0;
  default_pass_desc.viewport_size = {4, 4};
  default_pass_desc.clear_depth = true;
  default_pass_desc.depth_clear_value = 1.0f;
  default_pass_desc.clear_stencil = true;
  default_pass_desc.stencil_clear_value = 0;
  default_pass_desc.color_attachments.push_back({
      .attachment_index = 0,
      .clear = true,
      .clear_color = {0.0f, 0.0f, 0.0f, 1.0f},
  });
  cg::rhi::GetDevice().BeginRenderPass(default_pass_desc);
  cg::rhi::GetDevice().EndRenderPass();

  cg::Texture source_color = MakeRgbaTexture(4, 4, {0u, 0u, 0u, 0u});
  cg::Texture source_depth = MakeDepthTexture(4, 4, 1.0f);
  cg::Texture source_stencil = MakeStencilTexture(4, 4, 0u);
  const uint32_t source_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc source_desc;
  source_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kColor,
      .attachment_index = 0,
      .texture = &source_color,
  });
  source_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kDepth,
      .attachment_index = 0,
      .texture = &source_depth,
  });
  source_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kStencil,
      .attachment_index = 0,
      .texture = &source_stencil,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(source_fbo, source_desc));

  cg::rhi::RenderPassDesc source_pass_desc;
  source_pass_desc.framebuffer = source_fbo;
  source_pass_desc.viewport_size = {4, 4};
  source_pass_desc.clear_depth = true;
  source_pass_desc.depth_clear_value = 0.6f;
  source_pass_desc.clear_stencil = true;
  source_pass_desc.stencil_clear_value = 9;
  source_pass_desc.color_attachments.push_back({
      .attachment_index = 0,
      .clear = true,
      .clear_color = {0.0f, 0.0f, 1.0f, 1.0f},
  });
  cg::rhi::GetDevice().BeginRenderPass(source_pass_desc);
  cg::rhi::GetDevice().EndRenderPass();

  cg::rhi::BlitFramebufferDesc blit_desc;
  blit_desc.read_framebuffer = source_fbo;
  blit_desc.draw_framebuffer = 0;
  blit_desc.size = {2, 2};
  blit_desc.mask = cg::rhi::ClearMask::kColor | cg::rhi::ClearMask::kDepth | cg::rhi::ClearMask::kStencil;
  blit_desc.filter = cg::rhi::FilterMode::kNearest;
  cg::rhi::GetDevice().BlitFramebuffer(blit_desc);

  std::array<uint8_t, 64> color_readback = {};
  cg::rhi::ReadPixelsDesc color_readback_desc;
  color_readback_desc.framebuffer = 0;
  color_readback_desc.attachment_type = cg::rhi::AttachmentType::kColor;
  color_readback_desc.read_buffer = cg::rhi::ReadBuffer::kBack;
  color_readback_desc.origin = {0, 0};
  color_readback_desc.size = {4, 4};
  color_readback_desc.format = cg::rhi::PixelFormat::kRGBA;
  color_readback_desc.type = cg::rhi::PixelType::kUInt8;
  cg::rhi::GetDevice().ReadPixels(color_readback_desc, color_readback.data());

  std::array<float, 16> depth_readback = {};
  cg::rhi::ReadPixelsDesc depth_readback_desc;
  depth_readback_desc.framebuffer = 0;
  depth_readback_desc.attachment_type = cg::rhi::AttachmentType::kDepth;
  depth_readback_desc.read_buffer = cg::rhi::ReadBuffer::kBack;
  depth_readback_desc.origin = {0, 0};
  depth_readback_desc.size = {4, 4};
  depth_readback_desc.format = cg::rhi::PixelFormat::kDepthComponent;
  depth_readback_desc.type = cg::rhi::PixelType::kFloat32;
  cg::rhi::GetDevice().ReadPixels(depth_readback_desc, depth_readback.data());

  std::array<uint8_t, 16> stencil_readback = {};
  cg::rhi::ReadPixelsDesc stencil_readback_desc;
  stencil_readback_desc.framebuffer = 0;
  stencil_readback_desc.attachment_type = cg::rhi::AttachmentType::kStencil;
  stencil_readback_desc.read_buffer = cg::rhi::ReadBuffer::kBack;
  stencil_readback_desc.origin = {0, 0};
  stencil_readback_desc.size = {4, 4};
  stencil_readback_desc.format = cg::rhi::PixelFormat::kStencilIndex;
  stencil_readback_desc.type = cg::rhi::PixelType::kUInt8;
  cg::rhi::GetDevice().ReadPixels(stencil_readback_desc, stencil_readback.data());

  for (int row = 0; row < 4; ++row) {
    for (int column = 0; column < 4; ++column) {
      const bool copied = row < 2 && column < 2;
      const size_t color_offset = static_cast<size_t>(row * 4 + column) * 4;
      EXPECT_EQ(color_readback[color_offset + 0], 0u);
      EXPECT_EQ(color_readback[color_offset + 1], 0u);
      EXPECT_EQ(color_readback[color_offset + 2], copied ? 255u : 0u);
      EXPECT_EQ(color_readback[color_offset + 3], 255u);
      EXPECT_FLOAT_EQ(depth_readback[static_cast<size_t>(row * 4 + column)], copied ? 0.6f : 1.0f);
      EXPECT_EQ(stencil_readback[static_cast<size_t>(row * 4 + column)], copied ? 9u : 0u);
    }
  }
}

TEST(vulkan_device_test, StandaloneDeviceSupportsStencilAttachmentBlit) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  cg::Texture source_stencil = MakeStencilTexture(2, 2, 0u);
  cg::Texture destination_stencil = MakeStencilTexture(2, 2, 0u);

  const uint32_t source_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc source_desc;
  source_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kStencil,
      .attachment_index = 0,
      .texture = &source_stencil,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(source_fbo, source_desc));

  cg::rhi::RenderPassDesc source_pass_desc;
  source_pass_desc.framebuffer = source_fbo;
  source_pass_desc.viewport_size = {2, 2};
  source_pass_desc.clear_stencil = true;
  source_pass_desc.stencil_clear_value = 19;
  cg::rhi::GetDevice().BeginRenderPass(source_pass_desc);
  cg::rhi::GetDevice().EndRenderPass();

  const uint32_t destination_fbo = cg::rhi::GetDevice().CreateFramebuffer();
  cg::rhi::FramebufferDesc destination_desc;
  destination_desc.attachments.push_back({
      .attachment_type = cg::rhi::AttachmentType::kStencil,
      .attachment_index = 0,
      .texture = &destination_stencil,
  });
  EXPECT_TRUE(cg::rhi::GetDevice().ConfigureFramebuffer(destination_fbo, destination_desc));

  cg::rhi::BlitFramebufferDesc blit_desc;
  blit_desc.read_framebuffer = source_fbo;
  blit_desc.draw_framebuffer = destination_fbo;
  blit_desc.size = {2, 2};
  blit_desc.mask = cg::rhi::ClearMask::kStencil;
  blit_desc.filter = cg::rhi::FilterMode::kNearest;
  cg::rhi::GetDevice().BlitFramebuffer(blit_desc);

  const std::vector<uint8_t> stencil_values = destination_stencil.GetData<uint8_t>();
  ASSERT_EQ(stencil_values.size(), 4u);
  for (uint8_t value : stencil_values) {
    EXPECT_EQ(value, 19u);
  }
}

TEST(vulkan_device_test, StandaloneDeviceRejectsDrawOutsideActiveRenderPass) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  EXPECT_DEATH(
      {
        auto device = cg::rhi::CreateVulkanDevice(false);
        if (device == nullptr) {
          std::abort();
        }
        cg::rhi::SetDevice(std::move(device));
        std::shared_ptr<cg::rhi::Program> program =
            cg::rhi::GetDevice().CreateProgram({
                .name = "draw_without_pass",
                .kind = cg::rhi::ProgramKind::kRender,
            });
        std::unique_ptr<cg::rhi::VertexArray> vertex_array = cg::rhi::GetDevice().CreateVertexArray();
        program->ApplyBindings({});
        cg::rhi::GetDevice().Draw({
            .kind = cg::rhi::DrawKind::kArrays,
            .topology = cg::rhi::PrimitiveTopology::kTriangles,
            .vertex_array = vertex_array.get(),
            .first = 0,
            .count = 3,
        });
      },
      ".*");
}

TEST(vulkan_device_test, StandaloneDeviceRejectsDrawWithoutProgramActivation) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  EXPECT_DEATH(
      {
        auto device = cg::rhi::CreateVulkanDevice(false);
        if (device == nullptr) {
          std::abort();
        }
        cg::rhi::SetDevice(std::move(device));
        std::unique_ptr<cg::rhi::VertexArray> vertex_array = cg::rhi::GetDevice().CreateVertexArray();
        cg::rhi::RenderPassDesc render_pass_desc;
        render_pass_desc.framebuffer = 0;
        render_pass_desc.viewport_size = glm::ivec2(1, 1);
        cg::rhi::GetDevice().BeginRenderPass(render_pass_desc);
        cg::rhi::GetDevice().Draw({
            .kind = cg::rhi::DrawKind::kArrays,
            .topology = cg::rhi::PrimitiveTopology::kTriangles,
            .vertex_array = vertex_array.get(),
            .first = 0,
            .count = 3,
        });
      },
      ".*");
}

TEST(vulkan_device_test, StandaloneDeviceRejectsDrawWithoutDrawDescVertexArray) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  EXPECT_DEATH(
      {
        auto device = cg::rhi::CreateVulkanDevice(false);
        if (device == nullptr) {
          std::abort();
        }
        cg::rhi::SetDevice(std::move(device));
        std::shared_ptr<cg::rhi::Program> program =
            cg::rhi::GetDevice().CreateProgram({
                .name = "draw_without_vao",
                .kind = cg::rhi::ProgramKind::kRender,
            });
        cg::rhi::RenderPassDesc render_pass_desc;
        render_pass_desc.framebuffer = 0;
        render_pass_desc.viewport_size = glm::ivec2(1, 1);
        cg::rhi::GetDevice().BeginRenderPass(render_pass_desc);
        program->ApplyBindings({});
        cg::rhi::GetDevice().Draw({
            .kind = cg::rhi::DrawKind::kArrays,
            .topology = cg::rhi::PrimitiveTopology::kTriangles,
            .first = 0,
            .count = 3,
        });
      },
      ".*");
}

TEST(vulkan_device_test, StandaloneDeviceRejectsIndexedDrawWithoutDrawDescIndexBuffer) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  EXPECT_DEATH(
      {
        auto device = cg::rhi::CreateVulkanDevice(false);
        if (device == nullptr) {
          std::abort();
        }
        cg::rhi::SetDevice(std::move(device));
        std::shared_ptr<cg::rhi::Program> program =
            cg::rhi::GetDevice().CreateProgram({
                .name = "draw_without_index_buffer",
                .kind = cg::rhi::ProgramKind::kRender,
            });
        std::unique_ptr<cg::rhi::VertexArray> vertex_array = cg::rhi::GetDevice().CreateVertexArray();
        cg::rhi::RenderPassDesc render_pass_desc;
        render_pass_desc.framebuffer = 0;
        render_pass_desc.viewport_size = glm::ivec2(1, 1);
        cg::rhi::GetDevice().BeginRenderPass(render_pass_desc);
        program->ApplyBindings({});
        cg::rhi::GetDevice().Draw({
            .kind = cg::rhi::DrawKind::kElements,
            .topology = cg::rhi::PrimitiveTopology::kTriangles,
            .vertex_array = vertex_array.get(),
            .count = 3,
        });
      },
      ".*");
}

TEST(vulkan_device_test, StandaloneDeviceAcceptsDrawWithExplicitGeometryBindings) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  std::shared_ptr<cg::rhi::Program> program =
      cg::rhi::GetDevice().CreateProgram({
          .name = "draw_ready",
          .kind = cg::rhi::ProgramKind::kRender,
      });
  ASSERT_TRUE(program != nullptr);
  std::unique_ptr<cg::rhi::VertexArray> vertex_array = cg::rhi::GetDevice().CreateVertexArray();
  ASSERT_TRUE(vertex_array != nullptr);
  std::unique_ptr<cg::rhi::Buffer> index_buffer = cg::rhi::GetDevice().CreateBuffer(cg::rhi::BufferType::kIndex);
  ASSERT_TRUE(index_buffer != nullptr);

  const std::array<uint32_t, 3> indices = {0u, 1u, 2u};
  index_buffer->SetData(sizeof(indices), indices.data(), cg::rhi::BufferUsage::kStatic);

  cg::rhi::RenderPassDesc render_pass_desc;
  render_pass_desc.framebuffer = 0;
  render_pass_desc.viewport_size = {1, 1};
  cg::rhi::GetDevice().BeginRenderPass(render_pass_desc);
  program->ApplyBindings({});

  EXPECT_NO_FATAL_FAILURE(cg::rhi::GetDevice().Draw({
      .kind = cg::rhi::DrawKind::kArrays,
      .topology = cg::rhi::PrimitiveTopology::kTriangles,
      .vertex_array = vertex_array.get(),
      .first = 0,
      .count = 3,
  }));
  EXPECT_NO_FATAL_FAILURE(cg::rhi::GetDevice().Draw({
      .kind = cg::rhi::DrawKind::kElements,
      .topology = cg::rhi::PrimitiveTopology::kTriangles,
      .vertex_array = vertex_array.get(),
      .index_buffer = index_buffer.get(),
      .count = 3,
  }));

  cg::rhi::GetDevice().EndRenderPass();
}

TEST(vulkan_device_test, StandaloneDeviceAcceptsDrawWithInlineGeometryBindings) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  std::shared_ptr<cg::rhi::Program> program = cg::rhi::GetDevice().CreateProgram({
      .name = "draw_inline_geometry",
      .kind = cg::rhi::ProgramKind::kRender,
  });
  ASSERT_TRUE(program != nullptr);
  std::unique_ptr<cg::rhi::VertexArray> vertex_array = cg::rhi::GetDevice().CreateVertexArray();
  ASSERT_TRUE(vertex_array != nullptr);
  std::unique_ptr<cg::rhi::Buffer> index_buffer = cg::rhi::GetDevice().CreateBuffer(cg::rhi::BufferType::kIndex);
  ASSERT_TRUE(index_buffer != nullptr);

  const std::array<uint32_t, 3> indices = {0u, 1u, 2u};
  index_buffer->SetData(sizeof(indices), indices.data(), cg::rhi::BufferUsage::kStatic);

  cg::rhi::RenderPassDesc render_pass_desc;
  render_pass_desc.framebuffer = 0;
  render_pass_desc.viewport_size = {1, 1};
  cg::rhi::GetDevice().BeginRenderPass(render_pass_desc);
  program->ApplyBindings({});

  EXPECT_NO_FATAL_FAILURE(cg::rhi::GetDevice().Draw({
      .kind = cg::rhi::DrawKind::kElements,
      .topology = cg::rhi::PrimitiveTopology::kTriangles,
      .vertex_array = vertex_array.get(),
      .index_buffer = index_buffer.get(),
      .count = 3,
  }));

  cg::rhi::GetDevice().EndRenderPass();
}

TEST(vulkan_device_test, StandaloneDeviceRejectsDispatchComputeWithoutProgramActivation) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  EXPECT_DEATH(
      {
        auto device = cg::rhi::CreateVulkanDevice(false);
        if (device == nullptr) {
          std::abort();
        }
        cg::rhi::SetDevice(std::move(device));
        (void)cg::rhi::GetDevice().CreateProgram({
            .name = "compute_without_use",
            .kind = cg::rhi::ProgramKind::kCompute,
        });
        cg::rhi::GetDevice().DispatchCompute({
            .workgroup_count = glm::uvec3(1, 1, 1),
        });
      },
      ".*");
}

TEST(vulkan_device_test, StandaloneDeviceRejectsDispatchComputeWithRenderProgramActive) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  EXPECT_DEATH(
      {
        auto device = cg::rhi::CreateVulkanDevice(false);
        if (device == nullptr) {
          std::abort();
        }
        cg::rhi::SetDevice(std::move(device));
        std::shared_ptr<cg::rhi::Program> program =
            cg::rhi::GetDevice().CreateProgram({
                .name = "render_program",
                .kind = cg::rhi::ProgramKind::kRender,
            });
        program->ApplyBindings({});
        cg::rhi::GetDevice().DispatchCompute({
            .workgroup_count = glm::uvec3(1, 1, 1),
        });
      },
      ".*");
}

TEST(vulkan_device_test, StandaloneDeviceRejectsDispatchComputeInsideActiveRenderPass) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  EXPECT_DEATH(
      {
        auto device = cg::rhi::CreateVulkanDevice(false);
        if (device == nullptr) {
          std::abort();
        }
        cg::rhi::SetDevice(std::move(device));
        std::shared_ptr<cg::rhi::Program> program =
            cg::rhi::GetDevice().CreateProgram({
                .name = "compute_in_pass",
                .kind = cg::rhi::ProgramKind::kCompute,
            });
        cg::rhi::RenderPassDesc render_pass_desc;
        render_pass_desc.framebuffer = 0;
        render_pass_desc.viewport_size = glm::ivec2(1, 1);
        cg::rhi::GetDevice().BeginRenderPass(render_pass_desc);
        program->ApplyBindings({});
        cg::rhi::GetDevice().DispatchCompute({
            .workgroup_count = glm::uvec3(1, 1, 1),
        });
      },
      ".*");
}

TEST(vulkan_device_test, StandaloneDeviceRejectsZeroWorkgroupComputeDispatch) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  EXPECT_DEATH(
      {
        auto device = cg::rhi::CreateVulkanDevice(false);
        if (device == nullptr) {
          std::abort();
        }
        cg::rhi::SetDevice(std::move(device));
        std::shared_ptr<cg::rhi::Program> program =
            cg::rhi::GetDevice().CreateProgram({
                .name = "zero_workgroup",
                .kind = cg::rhi::ProgramKind::kCompute,
            });
        program->ApplyBindings({});
        cg::rhi::GetDevice().DispatchCompute({
            .workgroup_count = glm::uvec3(0, 1, 1),
        });
      },
      ".*");
}

TEST(vulkan_device_test, StandaloneDeviceAcceptsDispatchComputeWithComputeProgramActive) {
  auto device = cg::rhi::CreateVulkanDevice(false);
  ASSERT_TRUE(device != nullptr);
  cg::rhi::SetDevice(std::move(device));
  ScopedDeviceReset scoped_reset;

  std::shared_ptr<cg::rhi::Program> program =
      cg::rhi::GetDevice().CreateProgram({
          .name = "compute_ready",
          .kind = cg::rhi::ProgramKind::kCompute,
      });
  ASSERT_TRUE(program != nullptr);

  program->ApplyBindings({});
  EXPECT_NO_FATAL_FAILURE(cg::rhi::GetDevice().DispatchCompute({
      .workgroup_count = glm::uvec3(2, 3, 1),
      .barrier = cg::rhi::MemoryBarrier::kAll,
  }));
}

}  // namespace
