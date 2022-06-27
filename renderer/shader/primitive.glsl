#include "renderer/shader/definition.glsl"
#include "renderer/shader/geometry.glsl"

struct Primitive {
  AABB aabb;
  Triangle triangle;
  vec3 normal;
  int material_index;
};

layout (std430, binding = SSBO_PRIMITIVE_REPO) buffer PrimitiveRepo {
  Primitive primitive_repo[];
};