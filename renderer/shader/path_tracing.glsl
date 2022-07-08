#include "renderer/shader/light.glsl"
#include "renderer/shader/random.glsl"
#include "renderer/shader/sample.glsl"
#include "renderer/shader/scene.glsl"

//! #include certain FR function before this file. Such as fr/BRDF.glsl

// https://www.bilibili.com/video/BV1X7411F744?p=16 00:59:00
vec4 path_tracing(vec4 radiance, Ray ray_iter, vec3 lo_normal, int max_depth, float rr) {
  const float kPi = 3.1415926;
  const float kBias = 0.0001;
  vec4 res = vec4(0, 0, 0, 1);
  radiance = dot(lo_normal, ray_iter.direction) * radiance;
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

// https://www.bilibili.com/video/BV1X7411F744?p=16 01:12:12
vec4 path_tracing_v2(vec4 radiance, Ray ray_iter, vec3 lo_normal_iter, int max_depth, float rr) {
  const float kPi = 3.1415926;
  const float kBias = 0.0001;
  vec4 res = vec4(0, 0, 0, 1);

  vec4 direct_lightings[20];
  vec4 shade[20];
  vec4 indirect_lighting_factors[20];
  for (int i = 0; i < 20; ++i) {
    direct_lightings[i] = vec4(0, 0, 0, 0);
    shade[i] = vec4(1, 1, 1, 1);
    indirect_lighting_factors[i] = vec4(0, 0, 0, 0);
  }

  int depth = 0;
  while(depth < max_depth) {
    float pdf_light = 1 / primitive_light_area;
    SampleLightResult sample_light_result = SampleLight();
    Ray directional_ray = Ray(ray_iter.position, normalize(sample_light_result.position - ray_iter.position));
    float lo_cosine = max(dot(directional_ray.direction, lo_normal_iter), 0);
    float li_cosine = max(dot(-directional_ray.direction, sample_light_result.normal), 0);
    RaySceneResult directional_result = RaySceneRaw(directional_ray);
    if (directional_result.hitted && directional_result.primitive_index == sample_light_result.primitive_index) {
      Material material = material_repo[directional_result.primitive_index];
      direct_lightings[depth] = radiance * lo_cosine * li_cosine / pdf_light;
    }

    if (Random() > rr) { break; }
    RaySceneResult indirectional_result = RaySceneRaw(ray_iter);
    if (indirectional_result.hitted == false) { break; }
    Material material = PrimitiveMaterial(indirectional_result.primitive_index);
    vec3 N = indirectional_result.normal;
    const float pdf = 1 / (2 * kPi);
    if (IsLight(indirectional_result.primitive_index)) { break; }
    vec3 Li = SampleUnitHemisphereDir(N);
    vec4 f_r = FR(-ray_iter.direction, Li, indirectional_result.normal, material);
    float cosine = max(dot(Li, N), 0.0);
    ray_iter = Ray(indirectional_result.position + kBias * Li, Li);
    indirect_lighting_factors[depth] = f_r * cosine / pdf / rr;
    lo_normal_iter = N;
    depth++;
  }
  for (int i = depth - 1; i >= 0; --i) {
    shade[i] = direct_lightings[i] + shade[i + 1] * indirect_lighting_factors[i];
  }
  return shade[0];
}

struct FromRasterization {
  vec3 Li;
  vec3 Lo;
  vec3 position;
  vec3 normal;
  int primitive_index;
};

vec4 path_tracing_from_rasterization(FromRasterization param, bool v2) {
  // vec4 f_r = FR(param.Li, param.Lo, param.normal, PrimitiveMaterial(param.primitive_index));
  vec4 f_r = MaterialDiffuse(PrimitiveMaterial(param.primitive_index));
  if (IsLight(param.primitive_index)) { return f_r; }
  Ray ray = Ray(param.position, param.Lo);
  vec4 radiance = f_r;
  if (v2) {
    return path_tracing_v2(f_r, ray, param.normal, 5, 0.9);
  } else {
    return path_tracing(f_r, ray, param.normal, 5, 0.9);
  }
}

struct FromCamera {
  Ray ray;
  vec3 normal;
};

vec4 path_tracing_from_camera(FromCamera param, bool v2) {
  if (v2) {
    return path_tracing_v2(vec4(1, 1, 1, 1), param.ray, param.normal, 5, 0.9);
  } else {
    return path_tracing(vec4(1, 1, 1, 1), param.ray, param.normal, 5, 0.9);
  }
}