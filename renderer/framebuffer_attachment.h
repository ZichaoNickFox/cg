#pragma once

#include <optional>
#include <string>

#include <glm/glm.hpp>

#include "renderer/color.h"
#include "renderer/gl.h"
#include "renderer/texture.h"

namespace renderer {
struct FramebufferAttachment {
  enum Type {
    kColor = 0,
    kDepth,
    kStencil,
    AttachmentTypeNum,
  };
  Type type;
  std::string name;
  Texture::Meta texture_meta;
  uint32_t multi_sample_num = 1;

  GLuint GetAttachmentBase() const;
};

static const FramebufferAttachment kAttachmentColor = {
    FramebufferAttachment::kColor, "color", {Texture::kTexture2D, -1, -1, 4, true, 1, 1, GL_RGBA32F,
    GL_RGBA, GL_FLOAT, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER}};
static const FramebufferAttachment kAttachmentPositionWS = {
    FramebufferAttachment::kColor, "position_ws", {Texture::kTexture2D, -1, -1, 4, true, 1, 1, GL_RGBA32F,
    GL_RGBA, GL_FLOAT, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER}};
static const FramebufferAttachment kAttachmentPositionVS = {
    FramebufferAttachment::kColor, "position_vs", {Texture::kTexture2D, -1, -1, 4, true, 1, 1, GL_RGBA32F,
    GL_RGBA, GL_FLOAT, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER}};
static const FramebufferAttachment kAttachmentNormalVS = {
    FramebufferAttachment::kColor, "normal_vs", {Texture::kTexture2D, -1, -1, 4, true, 1, 1, GL_RGBA32F,
    GL_RGBA, GL_FLOAT, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER}};
static const FramebufferAttachment kAttachmentNormalWS = {
    FramebufferAttachment::kColor, "normal_ws",{Texture::kTexture2D, -1, -1, 4, true, 1, 1, GL_RGBA32F,
    GL_RGBA, GL_FLOAT, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER}};
static const FramebufferAttachment kAttachmentSurfaceNormalWS = {
    FramebufferAttachment::kColor, "surface_normal_ws",{Texture::kTexture2D, -1, -1, 4, true, 1, 1, GL_RGBA32F,
    GL_RGBA, GL_FLOAT, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER}};
static const FramebufferAttachment kAttachmentTexcoord = {
    FramebufferAttachment::kColor, "texcoord", {Texture::kTexture2D, -1, -1, 2, true, 1, 1, GL_RG32F,
    GL_RG, GL_FLOAT, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER}};
static const FramebufferAttachment kAttachmentDepth = {
    FramebufferAttachment::kDepth, "depth", {Texture::kTexture2D, -1, -1, 1, true, 1, 1, GL_DEPTH_COMPONENT32F,
    GL_DEPTH_COMPONENT, GL_FLOAT, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER}};
static const FramebufferAttachment kAttachmentStencil = {
    FramebufferAttachment::kDepth, "stencil", {Texture::kTexture2D, -1, -1, 1, true, 1, 1, GL_DEPTH_COMPONENT32F,
    GL_DEPTH_COMPONENT, GL_FLOAT, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER}};
} // namespace renderer