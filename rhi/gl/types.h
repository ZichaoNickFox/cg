#pragma once

#include "renderer/gl.h"
#include "rhi/types.h"

namespace cg::rhi {

inline GLenum ToGLPrimitiveTopology(PrimitiveTopology topology) {
  switch (topology) {
    case PrimitiveTopology::kTriangles:
      return GL_TRIANGLES;
    case PrimitiveTopology::kTriangleStrip:
      return GL_TRIANGLE_STRIP;
    case PrimitiveTopology::kLines:
      return GL_LINES;
    case PrimitiveTopology::kLineStrip:
      return GL_LINE_STRIP;
  }
  CGCHECK(false) << "Unsupported primitive topology";
  return 0;
}

inline GLenum ToGLTextureFormat(TextureFormat format) {
  switch (format) {
    case TextureFormat::kRGBA8:
      return GL_RGBA8;
    case TextureFormat::kRGBA32F:
      return GL_RGBA32F;
    case TextureFormat::kRG32F:
      return GL_RG32F;
    case TextureFormat::kR32UI:
      return GL_R32UI;
    case TextureFormat::kDepth32F:
      return GL_DEPTH_COMPONENT32F;
    case TextureFormat::kUnknown:
      break;
  }
  CGCHECK(false) << "Unsupported texture format";
  return 0;
}

inline GLenum ToGLPixelFormat(PixelFormat format) {
  switch (format) {
    case PixelFormat::kRGBA:
      return GL_RGBA;
    case PixelFormat::kRG:
      return GL_RG;
    case PixelFormat::kRedInteger:
      return GL_RED_INTEGER;
    case PixelFormat::kDepthComponent:
      return GL_DEPTH_COMPONENT;
    case PixelFormat::kUnknown:
      break;
  }
  CGCHECK(false) << "Unsupported pixel format";
  return 0;
}

inline GLenum ToGLPixelType(PixelType type) {
  switch (type) {
    case PixelType::kUInt8:
      return GL_UNSIGNED_BYTE;
    case PixelType::kFloat32:
      return GL_FLOAT;
    case PixelType::kUInt32:
      return GL_UNSIGNED_INT;
    case PixelType::kUnknown:
      break;
  }
  CGCHECK(false) << "Unsupported pixel type";
  return 0;
}

inline GLenum ToGLFilterMode(FilterMode filter) {
  switch (filter) {
    case FilterMode::kNearest:
      return GL_NEAREST;
    case FilterMode::kLinear:
      return GL_LINEAR;
    case FilterMode::kLinearMipmapLinear:
      return GL_LINEAR_MIPMAP_LINEAR;
  }
  CGCHECK(false) << "Unsupported filter mode";
  return 0;
}

inline GLenum ToGLWrapMode(WrapMode wrap) {
  switch (wrap) {
    case WrapMode::kRepeat:
      return GL_REPEAT;
    case WrapMode::kClampToBorder:
      return GL_CLAMP_TO_BORDER;
    case WrapMode::kClampToEdge:
      return GL_CLAMP_TO_EDGE;
  }
  CGCHECK(false) << "Unsupported wrap mode";
  return 0;
}

inline GLenum ToGLCullMode(CullMode mode) {
  switch (mode) {
    case CullMode::kBack:
      return GL_BACK;
    case CullMode::kFront:
      return GL_FRONT;
    case CullMode::kFrontAndBack:
      return GL_FRONT_AND_BACK;
  }
  CGCHECK(false) << "Unsupported cull mode";
  return 0;
}

inline GLenum ToGLFrontFace(FrontFace winding) {
  switch (winding) {
    case FrontFace::kClockwise:
      return GL_CW;
    case FrontFace::kCounterClockwise:
      return GL_CCW;
  }
  CGCHECK(false) << "Unsupported front face";
  return 0;
}

inline GLbitfield ToGLClearMask(ClearMask mask) {
  GLbitfield bits = 0;
  if (HasAnyFlag(mask, ClearMask::kColor)) {
    bits |= GL_COLOR_BUFFER_BIT;
  }
  if (HasAnyFlag(mask, ClearMask::kDepth)) {
    bits |= GL_DEPTH_BUFFER_BIT;
  }
  if (HasAnyFlag(mask, ClearMask::kStencil)) {
    bits |= GL_STENCIL_BUFFER_BIT;
  }
  return bits;
}

}  // namespace cg::rhi
