#pragma once

#include <optional>
#include <string>

#include <glm/glm.hpp>

#include "renderer/gl.h"

namespace renderer {
struct FramebufferAttachment {
  enum AttachmentType {
    kColor = 0,
    kDepth,
    kStencil,
    AttachmentTypeNum,
  };
  AttachmentType type;
  std::string name;
  GLuint texture_format;
  GLuint texture_type;
  GLuint texture_internal_type;
  GLuint texture_param_min_filter;
  GLuint texture_param_mag_filter;
  GLuint texture_param_wrap_s;
  GLuint texture_param_wrap_t;
  uint32_t multi_sample_num;
  glm::vec4 clear_color;

  GLuint GetAttachmentBase() const;
};

static const FramebufferAttachment kAttachmentColor = {
    FramebufferAttachment::kColor, "color", GL_RGBA, GL_FLOAT, GL_RGBA32F,
    GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, 1, glm::vec4(0.45f, 0.55f, 0.60f, 1.00f)};
static const FramebufferAttachment kAttachmentPositionWS = {
    FramebufferAttachment::kColor, "position_ws", GL_RGBA, GL_FLOAT, GL_RGBA32F,
    GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, 1, glm::vec4(0.45f, 0.55f, 0.60f, 1.00f)};
static const FramebufferAttachment kAttachmentPositionVS = {
    FramebufferAttachment::kColor, "position_vs", GL_RGBA, GL_FLOAT, GL_RGBA32F,
    GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, 1, glm::vec4(0.45f, 0.55f, 0.60f, 1.00f)};
static const FramebufferAttachment kAttachmentNormalVS = {
    FramebufferAttachment::kColor, "normal_vs", GL_RGBA, GL_FLOAT, GL_RGBA32F,
    GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, 1, glm::vec4(0.45f, 0.55f, 0.60f, 1.00f)};
static const FramebufferAttachment kAttachmentNormalWS = {
    FramebufferAttachment::kColor, "normal_ws", GL_RGBA, GL_FLOAT, GL_RGBA32F,
    GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, 1, glm::vec4(0.45f, 0.55f, 0.60f, 1.00f)};
static const FramebufferAttachment kAttachmentTexcoord = {
    FramebufferAttachment::kColor, "texcoord", GL_RGBA, GL_FLOAT, GL_RGBA32F,
    GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, 1, glm::vec4(0.45f, 0.55f, 0.60f, 1.00f)};
static const FramebufferAttachment kAttachmentDepth = {
    FramebufferAttachment::kDepth, "depth", GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_COMPONENT32F,
    GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, 1, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)};
static const FramebufferAttachment kAttachmentStencil = {
    FramebufferAttachment::kDepth, "stencil", GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_COMPONENT32F,
    GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, 1, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)};
} // namespace renderer