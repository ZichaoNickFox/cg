#include "renderer/shader/light.glsl"
#include "renderer/shader/random.glsl"
#include "renderer/shader/sample.glsl"
#include "renderer/shader/scene.glsl"

//! #include certain FR function before this file. Such as fr/BRDF.glsl

struct PathTracingInitialRay {
  Ray ray;
  vec4 radiance;
  vec3 normal;
  bool from_light;
};

// If first ray is rasterized, radiance is that color. RayFromLight may be light
// Else radiance is vec4(1, 1, 1, 1). RayFromLight = false
// max_depth : 5
// rr : 0.9
// https://www.bilibili.com/video/BV1X7411F744?p=16 0:58:08
vec4 path_tracing(PathTracingInitialRay initial_ray, int max_depth, float rr) {
  if (initial_ray.from_light) { return initial_ray.radiance; }

  const float kPi = 3.1415926;
  const float kBias = 0.0001;

  vec4 res = vec4(0, 0, 0, 1);
  Ray ray_iter = initial_ray.ray;
  vec4 radiance = initial_ray.radiance;

  while(bool(max_depth--)) {
    if (Random() > rr) { break; }

    RaySceneResult result = RaySceneRaw(ray_iter);
    if (result.hitted == false) { return res; }

    Primitive primitive = primitive_repo[result.primitive_index];
    Material material = material_repo[PrimitiveMaterialIndex(primitive)];
    vec3 N = result.normal;

    const float pdf = 1 / (2 * kPi);
    if (IsLight(result.primitive_index)) {
      float cosine = max(dot(-ray_iter.direction, N), 0);
      res = radiance * MaterialDiffuse(material) * cosine / pdf / rr;
      break;
    } else {
      vec3 direction_ws = SampleUnitHemisphereDir(N);
      vec4 f_r = FR(-ray_iter.direction, direction_ws, result.normal, material);
      float cosine = max(dot(direction_ws, N), 0.0);
      radiance = radiance * f_r * cosine / pdf / rr;
      ray_iter = Ray(result.position + kBias * direction_ws, direction_ws);
    }
  }
  return res;
}

/*
// https://www.bilibili.com/video/BV1X7411F744?p=16 01:12:12
vec4 path_tracing_v2(PathTracingInitialRay initial_ray, int max_depth, float rr) {
  if (initial_ray.from_light) { return initial_ray.radiance; }

  const float kPi = 3.1415926;
  const float kBias = 0.0001;

  vec4 res = vec4(0, 0, 0, 1);
  Ray ray_iter = initial_ray.ray;

  vec4 radiance = initial_ray.radiance;
  vec3 lo_normal = initial_ray.normal;

  vec4 directional_lights[20];
  vec4 shade[20];
  vec4 indirectional_light_factors[20];

  int depth = 0;
  while(depth < max_depth) {
    float pdf_light = 1 / primitive_light_area;
    SampleLightResult sample_light_result = SampleLight();
    Ray directional_ray = Ray(ray_iter.position, normalize(sample_light_result.position - ray_iter.position));
    float lo_cosine = max(dot(directional_ray.direction, lo_normal), 0);
    float li_cosine = max(dot(-directional_ray.direction, sample_light_result.normal), 0);
    RaySceneResult directional_result = RaySceneRaw(directional_ray);
    if (directional_result.hitted && directional_result.primitive_index == directional_result.primitive_index) {
      Material material = material_repo[directional_result.primitive_index];
      directional_lights[depth] = MaterialDiffuse(material) * lo_cosine * li_cosine / pdf_light;
    }

    if (Random() > rr) { break; }
    RaySceneResult indirectional_result = RaySceneRaw(ray_iter);
    if (indirectional_result.hitted == false) { break; }
    Primitive primitive = primitive_repo[indirectional_result.primitive_index];
    Material material = material_repo[PrimitiveMaterialIndex(primitive)];
    vec3 N = indirectional_result.normal;
    const float pdf = 1 / (2 * kPi);
    if (IsLight(indirectional_result.primitive_index)) { break; }
    vec3 direction_ws = SampleUnitHemisphereDir(N);
    vec4 f_r = FR(-ray_iter.direction, direction_ws, indirectional_result.normal, material);
    float cosine = max(dot(direction_ws, N), 0.0);
    radiance = radiance * f_r * cosine / pdf / rr;
    ray_iter = Ray(indirectional_result.position + kBias * direction_ws, direction_ws);
    depth++;
  }
  for (int i = depth - 1; i >= 0; --i) {
    shade[i] = directional_lights[i + 1] + shade[i + 1];
  }
  return shade[0];
}
*/