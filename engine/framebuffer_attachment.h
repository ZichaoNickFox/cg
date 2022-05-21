#pragma once

#include <optional>
#include <string>

#include <glm/glm.hpp>

#include "engine/gl.h"

namespace engine {
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
  uint32_t size_in_float;
  uint32_t size_in_byte;
  GLuint texture_param_min_filter;
  GLuint texture_param_mag_filter;
  GLuint texture_param_wrap_s;
  GLuint texture_param_wrap_t;
  uint32_t multi_sample_num;
  glm::vec4 clear_color;

  GLuint GetAttachmentBase() const;
};

static const char* kAttachmentNameColor = "color";
static const char* kAttachmentNamePosition = "position";
static const char* kAttachmentNameNormal = "normal";
static const char* kAttachmentNameTexcoord = "texcoord";
static const char* kAttachmentNameDepth = "depth";
static const char* kAttachmentNameStencil = "stencil";

static const FramebufferAttachment kAttachmentColor = {
    FramebufferAttachment::kColor, kAttachmentNameColor, GL_RGBA, GL_FLOAT, GL_RGBA32F,
    4, 16, GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT, 1, glm::vec4(0.45f, 0.55f, 0.60f, 1.00f)};
static const FramebufferAttachment kAttachmentPosition = {
    FramebufferAttachment::kColor, kAttachmentNamePosition, GL_RGBA, GL_FLOAT, GL_RGBA32F,
    4, 16, GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT, 1, glm::vec4(0.45f, 0.55f, 0.60f, 1.00f)};
static const FramebufferAttachment kAttachmentNormal = {
    FramebufferAttachment::kColor, kAttachmentNameNormal, GL_RGBA, GL_FLOAT, GL_RGBA32F,
    4, 16, GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT, 1, glm::vec4(0.45f, 0.55f, 0.60f, 1.00f)};
static const FramebufferAttachment kAttachmentTexcoord = {
    FramebufferAttachment::kColor, kAttachmentNameTexcoord, GL_RGBA, GL_FLOAT, GL_RGBA32F,
    4, 16, GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT, 1, glm::vec4(0.45f, 0.55f, 0.60f, 1.00f)};
static const FramebufferAttachment kAttachmentDepth = {
    FramebufferAttachment::kDepth, kAttachmentNameDepth, GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_COMPONENT32F,
    4, 4, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, 1, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)};
static const FramebufferAttachment kAttachmentStencil = {
    FramebufferAttachment::kDepth, kAttachmentNameStencil, GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_COMPONENT32F,
    4, 4, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, 1, glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)};
} // namespace engine