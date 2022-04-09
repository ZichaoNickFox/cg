#pragma once

#include "GL/glew.h"
#include <unordered_map>

enum class PassType {
  kNone,
  kForwardShading,
  kShadowMapping,
  kDeferredShadingGeometry,
  kDeferredShadingLighting,
};

struct Pass {
  PassType type;
  std::string shader_name;
  bool operator == (const Pass& other) const {
    return type == other.type && shader_name == other.shader_name;
  }
  bool operator != (const Pass& other) const { return !(*this == other); }
};
namespace pass {
static const Pass kPassNone = {PassType::kNone, ""};
static const Pass kPassForwardShading = {PassType::kForwardShading, "forward_shading"};
static const Pass kPassShadowMapping = {PassType::kShadowMapping, "shadow_mapping"};
static const Pass kPassDeferredShadingGeometry = {PassType::kDeferredShadingGeometry, "deferred_shading_geometry"};
static const Pass kPassDeferredShadingLighting = {PassType::kDeferredShadingLighting, "deferred_shading_lighting"};
}

struct MeshVertexComponent {
  std::string name;
  GLuint format;
  GLuint type;
  GLuint internal_type;
  uint32_t size_in_float;
  uint32_t size_in_byte;
};
static const std::unordered_map<std::string, MeshVertexComponent> kMeshVertexLayout = {
  {"position", {"position", GL_RGB, GL_FLOAT, GL_RGB32F, 3, 12}},
  {"normal", {"normal", GL_RGB, GL_FLOAT, GL_RGB32F, 3, 12}},
  {"texcoord", {"texcoord", GL_RG, GL_FLOAT, GL_RG32F, 2, 8}},
  {"tangent", {"tangent", GL_RGB, GL_FLOAT, GL_RGB32F, 3, 12}},
  {"bitangent", {"bitangent", GL_RGB, GL_FLOAT, GL_RGB32F, 3, 12}},
};

struct FrameBufferAttachment {
  std::string name;
  GLuint format;
  GLuint type;
  GLuint internal_type;
  uint32_t size_in_float;
  uint32_t size_in_byte;
  GLuint attachment_unit;
  bool is_draw_attachment_unit;
  GLuint texture_param_min_filter;
  GLuint texture_param_mag_filter;
  GLuint texture_param_wrap_s;
  GLuint texture_param_wrap_t;
};
static const std::vector<FrameBufferAttachment> kGBufferMRTLayout = {
  {"position", GL_RGBA, GL_FLOAT, GL_RGBA32F, 4, 16, GL_COLOR_ATTACHMENT0, true,
      GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT},
  {"normal", GL_RGBA, GL_FLOAT, GL_RGBA32F, 4, 16, GL_COLOR_ATTACHMENT1, true,
      GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT},
  {"texcoord", GL_RGBA, GL_FLOAT, GL_RGBA32F, 4, 16, GL_COLOR_ATTACHMENT2, true,
      GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT},
  {"frag_world_pos", GL_RGBA, GL_FLOAT, GL_RGBA32F, 4, 16, GL_COLOR_ATTACHMENT3, true,
      GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT},
  {"depth", GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_COMPONENT32F, 4, 4, GL_DEPTH_ATTACHMENT, false,
      GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE},
};