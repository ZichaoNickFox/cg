#include "renderer/shader/bvh.glsl"
#include "renderer/shader/geometry.glsl"

struct RaySceneResult {
  bool hitted;
  int primitive_index;
  float dist;
  vec3 normal;
  vec3 pos;
};

RaySceneResult RayScene(Ray ray, bool use_bvh) {
  RaySceneResult res;
  if (use_bvh) {
    RayBVHResult result = RayBVH(ray);
    res.hitted = result.hitted;
    res.primitive_index = result.primitive_index;
    res.dist = result.dist;
    res.normal = result.normal;
    res.pos = result.pos;
  } else {
    res.hitted = false;
    res.dist = FLT_MAX;
    for (int i = 0; i < primitive_repo_num; ++i) {
      Primitive primitive = primitive_repo[i];
      Triangle triangle = PrimitiveTriangle(primitive);
      RayTriangleResult result = RayTriangle(ray, triangle);
      if (result.hitted && result.dist < res.dist) {
        res.hitted = result.hitted;
        res.primitive_index = i;
        res.pos = result.pos;
        res.normal = result.normal;
        res.dist = result.dist;
      }
    }
  }
  return res;
}