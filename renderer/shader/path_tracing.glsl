#include "renderer/shader/light.glsl"
#include "renderer/shader/random.glsl"
#include "renderer/shader/sample.glsl"
#include "renderer/shader/scene.glsl"

//! #include certain FR function before this file. Such as fr/BRDF.glsl

// If first ray is rasterized, radiance is that color. RayFromLight may be light
// Else radiance is vec4(1, 1, 1, 1). RayFromLight = false
// max_depth : 5
// rr : 0.9
// https://www.bilibili.com/video/BV1X7411F744?p=16 0:58:08
vec4 path_tracing(Ray ray, vec4 radiance, bool ray_from_light, int max_depth, float rr) {
  if (ray_from_light) { return radiance; }

  const float kPi = 3.1415926;
  const float kBias = 0.0001;

  vec4 res = vec4(0, 0, 0, 1);
  Ray ray_iter = ray;

  while(bool(max_depth--)) {
    if (Random() > rr) { break; }

    RaySceneResult result = RaySceneRaw(ray_iter);
    if (result.hitted == false) { return res; }

    Primitive primitive = primitive_repo[result.primitive_index];
    Material material = material_repo[PrimitiveMaterialIndex(primitive)];
    vec3 N = result.normal;

    const float pdf = 1 / (2 * kPi);
    if (IsLight(result.primitive_index)) {
      float cosine = max(dot(-ray_iter.dir, N), 0);
      res = radiance * MaterialDiffuse(material) * cosine / pdf / rr;
      break;
    } else {
      vec3 dir_ws = SampleUnitHemisphereDir(N);
      vec4 f_r = FR(-ray_iter.dir, dir_ws, result.normal, material);
      float cosine = max(dot(dir_ws, N), 0.0);
      radiance = radiance * f_r * cosine / pdf / rr;
      ray_iter = Ray(result.pos + kBias * dir_ws, dir_ws);
    }
  }
  return res;
}

// https://www.bilibili.com/video/BV1X7411F744?p=16 01:12:12
vec4 path_tracing_v2(Ray ray, vec4 radiance, bool ray_from_light, int max_depth, float rr) {
  if (ray_from_light) { return radiance; }

  const float kPi = 3.1415926;
  const float kBias = 0.0001;

  vec4 res = vec4(0, 0, 0, 1);
  Ray ray_iter = ray;

  while(bool(max_depth--)) {
    if (Random() > rr) { break; }

    RaySceneResult result = RaySceneRaw(ray_iter);
    if (result.hitted == false) { return res; }

    Primitive primitive = primitive_repo[result.primitive_index];
    Material material = material_repo[PrimitiveMaterialIndex(primitive)];
    vec3 N = result.normal;
    const float pdf = 1 / (2 * kPi);
    if (!IsLight(result.primitive_index)) {
      vec3 dir_ws = SampleUnitHemisphereDir(N);
      vec4 f_r = FR(-ray_iter.dir, dir_ws, result.normal, material);
      float cosine = max(dot(dir_ws, N), 0.0);
      radiance = radiance * f_r * cosine / pdf / rr;
      ray_iter = Ray(result.pos + kBias * dir_ws, dir_ws);
    }
  }
  return res;
}
