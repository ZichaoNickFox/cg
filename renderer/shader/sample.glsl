#include "renderer/shader/definition.glsl"
// #include "renderer/shader/lut/distance_lut.glsl"
// #include "renderer/shader/lut/gaussian_lut.glsl"
#include "renderer/shader/light.glsl"
#include "renderer/shader/random.glsl"

// random x:[0,1] y[0,1] in uv
//---------------------------------------------------------------------

float RadicalInverse_VdC(uint bits) {
  bits = (bits << 16u) | (bits >> 16u);
  bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
  bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
  bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
  bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
  return float(bits) * 2.3283064365386963e-10;  // / 0x10000000
}

vec2 Hammersley(uint i, uint N) {
  return vec2(float(i) / float(N), RadicalInverse_VdC(i));
}

//---------------------------------------------------------------------

vec2 SpherePos2UV(vec3 local_pos) {
  // Normalize is need
  // Local_pos.y maybe [-0.5 - 0.5] depends on mesh
  // After normalize, local_pos.y would be [-1, 1]
  // Cube's normalized local_pos is sphere
  local_pos = normalize(local_pos);
  const float pi = 3.1415926;
  float u = atan(local_pos.z, local_pos.x);
  if (u < 0.0) {
    u = u + 2.0 * pi;
  }
  u = u / (2.0 * pi);
  float v = asin(local_pos.y);
  v = v / (pi / 2.0);
  v = (v + 1.0) / 2.0;
  return vec2(u, v);
}

// Sample nearing H direction, according to roughness and normal.
//---------------------------------------------------------------------
vec3 ImportancleSampleGGX(vec2 Xi, vec3 N, float roughness) {
  const float pi = 3.1415926;
  float a = roughness * roughness;
  float phi = 2.0 * pi * Xi.x;
  float cos_theta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
  float sin_theta = sqrt(1.0 - cos_theta * cos_theta);

  vec3 H;
  H.x = cos(phi) * sin_theta;
  H.y = sin(phi) * sin_theta;
  H.z = cos_theta;

  vec3 up = abs(N.z < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0));
  vec3 tangent = normalize(cross(up, N));
  vec3 bitangent = cross(N, tangent);

  vec3 sample_vec = H.x * tangent + H.y * bitangent + H.z * N;
  return normalize(sample_vec);
}

// Hemisphere random direction sample
vec3 SampleUnitHemisphereDir(vec3 normal) { 
  if (abs(normal.y - 1) < FLT_EPSILON) {
    normal = vec3(0.005, 0.99, 0.005);
  } else if (abs(normal.y + 1) < FLT_EPSILON) {
    normal = vec3(0.005, -0.99, -0.005);
  }

  vec3 x_ts_base = normalize(cross(vec3(0, 1, 0), normal));
  vec3 z_ts_base = normalize(cross(x_ts_base, normal));
  vec3 y_ts_base = normalize(normal);
  float x_ts = Random() * 2 - 1;
  float z_ts = Random() * 2 - 1;
  float y_ts = Random();
  vec3 dir = mat3(x_ts_base, y_ts_base, z_ts_base) * vec3(x_ts, y_ts, z_ts);
  dir = normalize(dir);
  return dir;
}

// Sphere random direction sample
vec3 SampleUnitSphereDir(vec3 normal) {
  float x = Random() * 2 - 1;
  float y = Random() * 2 - 1;
  float z = Random() * 2 - 1;
  return normalize(vec3(x, y ,z));
}

/*
// https://www.mathsisfun.com/data/standard-normal-distribution-table.html
float StandardGaussianPercent(float from, float to) {
  // TODO 
  return 0.0;
}

float SampleGaussianWeight(int sample_radius, float distance) {
  float index = min(distance, float(sample_radius));
  return gaussian_lut[sample_radius][int(index) * 10 - 1];
}
*/

void SampleLight() {

}