#include "renderer/shader/definition.glsl"
#include "renderer/shader/geometry.glsl"

layout (std430, binding = SSBO_PRIMITIVE_REPO) buffer PrimitiveRepo { Primitive primitives[]; };

struct Primitive {
  AABB aabb;
  Triangle triangle;
  vec3 normal;
  int material_index;
};