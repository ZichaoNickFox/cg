#pragma once

#include <cstdint>

namespace cg::rhi {

enum class PrimitiveTopology {
  kTriangles = 0,
  kTriangleStrip = 1,
  kLines = 2,
  kLineStrip = 3,
};

enum class TextureFormat {
  kUnknown = 0,
  kRGBA8 = 1,
  kRGBA32F = 2,
  kRG32F = 3,
  kR32UI = 4,
  kDepth32F = 5,
};

enum class PixelFormat {
  kUnknown = 0,
  kRGBA = 1,
  kRG = 2,
  kRedInteger = 3,
  kDepthComponent = 4,
};

enum class PixelType {
  kUnknown = 0,
  kUInt8 = 1,
  kFloat32 = 2,
  kUInt32 = 3,
};

enum class FilterMode {
  kNearest = 0,
  kLinear = 1,
  kLinearMipmapLinear = 2,
};

enum class WrapMode {
  kRepeat = 0,
  kClampToBorder = 1,
  kClampToEdge = 2,
};

enum class CullMode {
  kBack = 0,
  kFront = 1,
  kFrontAndBack = 2,
};

enum class FrontFace {
  kClockwise = 0,
  kCounterClockwise = 1,
};

enum class ClearMask : uint32_t {
  kNone = 0,
  kColor = 1u << 0,
  kDepth = 1u << 1,
  kStencil = 1u << 2,
};

inline constexpr ClearMask operator|(ClearMask lhs, ClearMask rhs) {
  return static_cast<ClearMask>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

inline constexpr ClearMask operator&(ClearMask lhs, ClearMask rhs) {
  return static_cast<ClearMask>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

inline constexpr bool HasAnyFlag(ClearMask value, ClearMask flag) {
  return static_cast<uint32_t>(value & flag) != 0;
}

}  // namespace cg::rhi
