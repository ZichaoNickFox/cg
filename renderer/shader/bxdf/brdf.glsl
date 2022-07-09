#include "renderer/shader/material.glsl"
#include "renderer/shader/pbr/pbr_geometry.glsl"
#include "renderer/shader/pbr/pbr_fresnel.glsl"
#include "renderer/shader/pbr/pbr_NDF.glsl"

// https://www.bilibili.com/video/BV1YK4y1T7yY?p=10 0:17:46
float BRDF_specular(vec3 Li, vec3 Lo, vec3 N, Material material) {
  float roughness = MaterialRoughness(material);
  float F = FresnelSchlink(Li, N, roughness);
  float G = GeometrySmith(N, Li, Lo, roughness);
  float NDF = D_GGX_TR(N, Li, Lo, roughness);
  float denominator = 4 * max(dot(N, Li), 0.0) * max(dot(N, Lo), 0.0) + 0.001;
  return F * G * NDF / denominator;
}

vec4 BRDF_diffuse(vec4 color) {
  const float pi = 3.1415926;
  return color / pi;
}

vec4 BXDF(vec3 Li, vec3 Lo, vec3 N, Material material) {
  // float f_r_specular = BRDF_specular(Li, Lo, N, material);
  vec4 f_r_diffuse = BRDF_diffuse(MaterialDiffuse(material));
  // vec4 f_r = 0.0 * f_r_specular + 1.0 * f_r_diffuse;
  return f_r_diffuse;
}