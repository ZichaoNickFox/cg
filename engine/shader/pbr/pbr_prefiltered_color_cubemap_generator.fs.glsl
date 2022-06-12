#include "engine/shader/version.glsl"

#include "engine/shader/random.glsl"
#include "engine/shader/sample.glsl"

uniform samplerCube environment_map;
uniform float roughness;

out vec4 FragColor;
in vec3 local_pos_;

const float pi = 3.1415926;

void main() {
  vec3 N = normalize(local_pos_);
  vec3 R = N;
  vec3 V = N;

  const int sample_num = 1024;
  float total_weight = 0.0;
  vec3 prefiltered_color = vec3(0.0);
  for (uint i = 0; i < sample_num; ++i) {
    vec2 Xi = Hammersley(i, sample_num);
    // TODO why sample H vector, rather than L or V
    vec3 H = ImportancleSampleGGX(Xi, N, roughness);
    vec3 L = normalize(2.0 * dot(V, H) * H - V);

    float n_dot_l = max(dot(N, L), 0.0);
    if (n_dot_l > 0.0) {
      prefiltered_color += texture(environment_map, L).rgb * n_dot_l;
      total_weight += n_dot_l;
    }
  }
  prefiltered_color = prefiltered_color / total_weight;
  FragColor = vec4(prefiltered_color, 1.0);
}