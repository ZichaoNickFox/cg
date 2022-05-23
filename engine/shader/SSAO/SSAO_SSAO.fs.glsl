#version 410 core

out vec3 out_frag_color;

in vec2 texcoord_;

uniform sampler2D ut_position_vs;
uniform sampler2D ut_normal_vs;
// uniform sampler2D ut_noise;

uniform vec3 u_samples_ts[64];
int kernel_size = 64;
float radius = 1.0;

uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
  vec3 position_vs = texture(ut_position_vs, texcoord_).xyz;
  vec3 normal_vs = texture(ut_normal_vs, texcoord_).xyz;
  vec3 up = (u_view * vec4(0, 1, 0, 1)).xyz;
  vec3 tangent = normalize(cross(up, normal_vs));
  vec3 bitangent = normalize(cross(normal_vs, tangent));
  mat3 TBN_vs = mat3(tangent, bitangent, normal_vs);
  float occlusion = 0.0;
  for (int i = 0; i < kernel_size; ++i) {
    vec3 sample_vs = TBN_vs * u_samples_ts[i];
    sample_vs = position_vs + sample_vs * radius;

    vec4 offset = vec4(sample_vs, 1.0);
    offset = u_projection * offset;
    offset.xyz /= offset.w;
    offset.xyz = offset.xyz * 0.5 + 0.5;

    float sample_depth = -texture(ut_position_vs, offset.xy).w;

    if (sample_depth > position_vs.z) {
      float range_check = smoothstep(0.0, 1.0, radius / abs(position_vs.z - sample_depth));
      occlusion += range_check;
    }
  }
  occlusion = 1.0 - (occlusion / kernel_size);
  out_frag_color = vec3(occlusion, occlusion, occlusion);
}
