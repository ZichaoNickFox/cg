#include "engine/shader/pbr/pbr_geometry.glsl"
#include "engine/shader/pbr/pbr_fresnel.glsl"
#include "engine/shader/pbr/pbr_NDF.glsl"

// https://www.bilibili.com/video/BV1YK4y1T7yY?p=10 0:17:46
float BRDF(vec3 V, vec3 L, vec3 N, float roughness) {
  float F = FresnelSchlink(V, N, roughness);
  float G = GeometrySmith(N, V, L, roughness);
  float NDF = D_GGX_TR(N, V, L, roughness);
  float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
  return F /* G*/ * NDF / denominator;
}