#include "renderer/shader/light.glsl"
#include "renderer/shader/random.glsl"
#include "renderer/shader/sample.glsl"
#include "renderer/shader/scene.glsl"

//! #include certain BXDF function before this file. Such as bxdf/brdf.glsl

// https://www.bilibili.com/video/BV1X7411F744?p=16 00:59:00
vec4 path_tracing(Ray ray_iter, int max_depth, float rr) {
  const float kPi = 3.1415926;
  const float kBias = 0.0001;
  vec4 res = vec4(0, 0, 0, 1);
  vec4 radiance = vec4(1, 1, 1, 1);

  while(bool(max_depth--)) {
    if (Random() > rr) { break; }

    RaySceneResult result = RaySceneRaw(ray_iter);
    if (result.hitted == false) { break; }

    Material material = PrimitiveMaterial(result.primitive_index);
    vec3 N = result.normal;
    
    const float pdf = 1 / (2 * kPi);
    if (IsLight(result.primitive_index)) {
      res = radiance * MaterialEmission(material) / rr;
      break;
    } else {
      vec3 direction_ws = SampleUnitHemisphereDir(N);
      vec4 f_r = BXDF(direction_ws, -ray_iter.direction, result.normal, material);
      float cosine = max(dot(direction_ws, N), 0.0);
      radiance = radiance * f_r * cosine / pdf / rr;
      ray_iter = Ray(result.position + kBias * direction_ws, direction_ws);
    }
  }
  return res;
}

// https://www.bilibili.com/video/BV1X7411F744?p=16 01:12:12
vec4 path_tracing_v2(Ray ray_iter, int max_depth, float rr) {
  const float kPi = 3.1415926;
  const float kBias = 0.0001;
  vec4 res = vec4(0, 0, 0, 1);

  vec4 direct_lightings[20];
  vec4 shade[20];
  vec4 indirect_lighting_factors[20];
  for (int i = 0; i < 20; ++i) {
    direct_lightings[i] = vec4(0, 0, 0, 0);
    shade[i] = vec4(0, 0, 0, 0);
    indirect_lighting_factors[i] = vec4(0, 0, 0, 0);
  }

  int depth = 0;
  vec3 normal_iter = ray_iter.direction;
  const float pdf = 1 / (2 * kPi);
  while(depth < max_depth) {
    RaySceneResult result = RaySceneRaw(ray_iter);
    if (result.hitted == false) { break; }
    Material material = PrimitiveMaterial(result.primitive_index);
    vec3 N = result.normal;
    if (IsLight(result.primitive_index)) {
      direct_lightings[depth] = MaterialEmission(material);
      break;
    }

    float pdf_light = 1 / primitive_light_area;
    SampleLightResult sample_light_result = SampleLight();
    vec3 light_vector = sample_light_result.position - result.position;
    vec3 normalized_light_vector = normalize(light_vector);
    Ray light_ray = Ray(result.position + kBias * normalized_light_vector, normalized_light_vector);
    float li_cosine = max(dot(sample_light_result.normal, -light_ray.direction), 0);
    float lo_cosine = max(dot(result.normal, light_ray.direction), 0);
    RaySceneResult directional_result = RaySceneRaw(light_ray);
    if (directional_result.hitted && directional_result.primitive_index == sample_light_result.primitive_index) {
      Material light_material = PrimitiveMaterial(sample_light_result.primitive_index);
      float distance = pow(length(light_vector), 2);
      vec4 f_r = BXDF(light_ray.direction, -ray_iter.direction, N, material);
      direct_lightings[depth] = f_r * MaterialEmission(light_material) * lo_cosine * li_cosine / pdf_light / distance;
    }

    if (Random() > rr) { break; }
    vec3 Li = SampleUnitHemisphereDir(N);
    vec4 f_r = BXDF(Li, -ray_iter.direction, N, material);
    float cosine = max(dot(Li, N), 0.0);
    ray_iter = Ray(result.position + kBias * Li, Li);
    indirect_lighting_factors[depth] = f_r * cosine / pdf / rr;
    normal_iter = N;
    depth++;
  }
  for (int i = depth; i >= 0; --i) {
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
  const float kBias = 0.0001;
  if (IsLight(param.primitive_index)) { return MaterialEmission(PrimitiveMaterial(param.primitive_index)); }
  const float kPi = 3.1415926;
  const float pdf = 1 / (2 * kPi);
  vec4 radiance = BXDF(param.Li, param.Lo, param.normal, PrimitiveMaterial(param.primitive_index)) / pdf;
  Ray ray = Ray(param.position + kBias * param.Lo, param.Lo);
  if (v2) {
    return path_tracing_v2(ray, 4, 0.9);
  } else {
    return path_tracing(ray, 4, 0.9);
  }
}

struct FromCamera {
  Ray ray;
  vec3 normal;
};

vec4 path_tracing_from_camera(FromCamera param, bool v2) {
  if (v2) {
    return path_tracing_v2(param.ray, 5, 0.9);
  } else {
    return path_tracing(param.ray, 5, 0.9);
  }
}