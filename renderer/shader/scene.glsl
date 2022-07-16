#include "renderer/shader/bvh.glsl"
#include "renderer/shader/geometry.glsl"
#include "renderer/shader/primitive.glsl"

struct RaySceneResult {
  bool hitted;
  int primitive_index;
  float distance;
  vec3 normal;
  vec3 position;
};

RaySceneResult RaySceneRaw(Ray ray) {
  RaySceneResult res;
  res.hitted = false;
  res.distance = FLT_MAX;
  for (int i = 0; i < primitive_repo_num; ++i) {
    Primitive primitive = primitive_repo[i];
    Triangle triangle = PrimitiveTriangle(primitive);
    RayTriangleResult result = RayTriangle(ray, triangle);
    if (result.hitted && result.distance < res.distance) {
      res.hitted = result.hitted;
      res.primitive_index = i;
      res.position = result.position;
      res.normal = result.normal;
      res.distance = result.distance;
    }
  }
  return res;
}

RaySceneResult RaySceneBVH(Ray ray) {
  RaySceneResult res;
  RayBVHResult result = RayBVH(ray);
  res.hitted = result.hitted;
  res.primitive_index = result.primitive_index;
  res.distance = result.distance;
  res.normal = result.normal;
  res.position = result.position;
  return res;
}

RaySceneResult RayScene(Ray ray) {
  // if (primitive_repo_num > 100) {
  //   return RaySceneBVH(ray);
  // } else {
    return RaySceneRaw(ray);
  // }
}