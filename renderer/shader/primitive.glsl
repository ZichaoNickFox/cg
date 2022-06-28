#include "renderer/shader/definition.glsl"
#include "renderer/shader/geometry.glsl"

struct Primitive {
  AABB aabb;
  Triangle triangle;
  vec4 normal_objectindex;
};

layout (std430, binding = SSBO_PRIMITIVE_REPO) buffer PrimitiveRepo {
  Primitive primitive_repo[];
};