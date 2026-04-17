#pragma once

#include <optional>
#include <cstdint>
#include <string>

#include <glm/glm.hpp>

#include "base/color.h"
#include "renderer/texture.h"

namespace cg {
struct FramebufferAttachment {
  enum Type {
    kColor = 0,
    kDepth,
    kStencil,
    AttachmentTypeNum,
  };
  enum ClearType {
    kClear = 0,
    kNoClear,
  };
  Type type;
  std::string name;
  Texture::Meta texture_meta;
  uint32_t multi_sample_num = 1;
  ClearType clear_type;
};

inline Texture::Meta MakeFramebufferTextureMeta(
    int channel_num, rhi::TextureFormat format, rhi::PixelFormat pixel_format, rhi::PixelType pixel_type) {
  Texture::Meta meta;
  meta.type = Texture::kTexture2D;
  meta.width = -1;
  meta.height = -1;
  meta.channel_num = channel_num;
  meta.hdr = true;
  meta.level_num = 1;
  meta.depth = 1;
  meta.format = format;
  meta.pixel_format = pixel_format;
  meta.pixel_type = pixel_type;
  meta.min_filter = rhi::FilterMode::kNearest;
  meta.mag_filter = rhi::FilterMode::kNearest;
  meta.wrap_s = rhi::WrapMode::kClampToBorder;
  meta.wrap_t = rhi::WrapMode::kClampToBorder;
  return meta;
}

static const FramebufferAttachment kAttachmentColor = {
    FramebufferAttachment::kColor, "color",
    MakeFramebufferTextureMeta(4, rhi::TextureFormat::kRGBA32F, rhi::PixelFormat::kRGBA, rhi::PixelType::kFloat32),
    1, FramebufferAttachment::kClear};
static const FramebufferAttachment kAttachmentColor1 = {
    FramebufferAttachment::kColor, "color_1",
    MakeFramebufferTextureMeta(4, rhi::TextureFormat::kRGBA32F, rhi::PixelFormat::kRGBA, rhi::PixelType::kFloat32),
    1, FramebufferAttachment::kClear};
static const FramebufferAttachment kAttachmentColorNoClear = {
    FramebufferAttachment::kColor, "color_noclear",
    MakeFramebufferTextureMeta(4, rhi::TextureFormat::kRGBA32F, rhi::PixelFormat::kRGBA, rhi::PixelType::kFloat32),
    1, FramebufferAttachment::kNoClear};
static const FramebufferAttachment kAttachmentColor1NoClear = {
    FramebufferAttachment::kColor, "color_1_noclear",
    MakeFramebufferTextureMeta(4, rhi::TextureFormat::kRGBA32F, rhi::PixelFormat::kRGBA, rhi::PixelType::kFloat32),
    1, FramebufferAttachment::kNoClear};
static const FramebufferAttachment kAttachmentEmission = {
    FramebufferAttachment::kColor, "emission",
    MakeFramebufferTextureMeta(4, rhi::TextureFormat::kRGBA32F, rhi::PixelFormat::kRGBA, rhi::PixelType::kFloat32),
    1, FramebufferAttachment::kClear};
static const FramebufferAttachment kAttachmentPositionWS = {
    FramebufferAttachment::kColor, "position_ws",
    MakeFramebufferTextureMeta(4, rhi::TextureFormat::kRGBA32F, rhi::PixelFormat::kRGBA, rhi::PixelType::kFloat32),
    1, FramebufferAttachment::kClear};
static const FramebufferAttachment kAttachmentPositionWS1 = {
    FramebufferAttachment::kColor, "position_ws_1",
    MakeFramebufferTextureMeta(4, rhi::TextureFormat::kRGBA32F, rhi::PixelFormat::kRGBA, rhi::PixelType::kFloat32),
    1, FramebufferAttachment::kClear};
static const FramebufferAttachment kAttachmentPositionVS = {
    FramebufferAttachment::kColor, "position_vs",
    MakeFramebufferTextureMeta(4, rhi::TextureFormat::kRGBA32F, rhi::PixelFormat::kRGBA, rhi::PixelType::kFloat32),
    1, FramebufferAttachment::kClear};
static const FramebufferAttachment kAttachmentNormalVS = {
    FramebufferAttachment::kColor, "normal_vs",
    MakeFramebufferTextureMeta(4, rhi::TextureFormat::kRGBA32F, rhi::PixelFormat::kRGBA, rhi::PixelType::kFloat32),
    1, FramebufferAttachment::kClear};
static const FramebufferAttachment kAttachmentNormalWS = {
    FramebufferAttachment::kColor, "normal_ws",
    MakeFramebufferTextureMeta(4, rhi::TextureFormat::kRGBA32F, rhi::PixelFormat::kRGBA, rhi::PixelType::kFloat32),
    1, FramebufferAttachment::kClear};
static const FramebufferAttachment kAttachmentSurfaceNormalWS = {
    FramebufferAttachment::kColor, "surface_normal_ws",
    MakeFramebufferTextureMeta(4, rhi::TextureFormat::kRGBA32F, rhi::PixelFormat::kRGBA, rhi::PixelType::kFloat32),
    1, FramebufferAttachment::kClear};
static const FramebufferAttachment kAttachmentTexcoord = {
    FramebufferAttachment::kColor, "texcoord",
    MakeFramebufferTextureMeta(2, rhi::TextureFormat::kRG32F, rhi::PixelFormat::kRG, rhi::PixelType::kFloat32),
    1, FramebufferAttachment::kClear};
static const FramebufferAttachment kAttachmentDepth = {
    FramebufferAttachment::kDepth, "depth",
    MakeFramebufferTextureMeta(1, rhi::TextureFormat::kDepth32F, rhi::PixelFormat::kDepthComponent, rhi::PixelType::kFloat32),
    1, FramebufferAttachment::kClear};
static const FramebufferAttachment kAttachmentStencil = {
    FramebufferAttachment::kDepth, "stencil",
    MakeFramebufferTextureMeta(1, rhi::TextureFormat::kDepth32F, rhi::PixelFormat::kDepthComponent, rhi::PixelType::kFloat32),
    1, FramebufferAttachment::kClear};
static const FramebufferAttachment kAttachmentTest = {
    FramebufferAttachment::kColor, "test",
    MakeFramebufferTextureMeta(4, rhi::TextureFormat::kRGBA32F, rhi::PixelFormat::kRGBA, rhi::PixelType::kFloat32),
    1, FramebufferAttachment::kClear};
static const FramebufferAttachment kAttachmentPrimitiveIndex = {
    FramebufferAttachment::kColor, "primitive_index",
    MakeFramebufferTextureMeta(1, rhi::TextureFormat::kR32UI, rhi::PixelFormat::kRedInteger, rhi::PixelType::kUInt32),
    1, FramebufferAttachment::kClear};
} // namespace cg
