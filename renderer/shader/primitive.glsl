#include "renderer/shader/definition.glsl"
#include "renderer/shader/geometry.glsl"

struct Primitive {
  vec4 a_materialIndex;
  vec4 b;
  vec4 c;
};

uniform int primitive_repo_num;
layout (std430, binding = SSBO_PRIMITIVE_REPO) buffer PrimitiveRepo {
  Primitive primitive_repo[];
};

int PrimitiveMaterialIndex(Primitive primitive) {
  return int(primitive.a_materialIndex.w);
}

Triangle PrimitiveTriangle(Primitive primitive) {
  return Triangle(vec4(primitive.a_materialIndex.xyz, 0.0), primitive.b, primitive.c);
};

vec3 PrimitiveNormal(Primitive primitive) {
  return normalize(TriangleNormal(PrimitiveTriangle(primitive)));
}