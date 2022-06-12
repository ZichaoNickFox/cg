#include "engine/shader/version.glsl"

#include "engine/shader/convert.glsl"
#include "engine/shader/depth.glsl"

out vec3 out_frag_color;

in vec2 texcoord_;

uniform sampler2D ut_position_vs;
uniform sampler2D ut_normal_vs;
uniform sampler2D ut_noise;

uniform vec3 u_samples_ts[64];
int kernel_size = 64;
float radius = 1.0;

uniform vec2 u_viewport_size;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
  vec3 position_vs = texture(ut_position_vs, texcoord_).xyz;
  vec3 normal_vs = texture(ut_normal_vs, texcoord_).xyz;

  vec3 random_vec = texture(ut_noise, texcoord_ * u_viewport_size).xyz;
  vec3 tangent_vs = normalize(random_vec - normal_vs * dot(random_vec, normal_vs));
//  vec3 up = vec3(0, 1, 0);
//  tangent_vs = normalize(cross(up, normal_vs));
  vec3 bitangent_vs = normalize(cross(normal_vs, tangent_vs));
  mat3 TBN_vs = mat3(tangent_vs, bitangent_vs, normal_vs);

  float depth_vs = GetLinearDepthFromVS(position_vs.z);
  float occlusion = 0.0;
  for (int i = 0; i < 64; ++i) {
    vec3 sample_offset_vs = TBN_vs * u_samples_ts[i];
    vec3 sample_vs = position_vs.xyz + sample_offset_vs * radius;

    vec4 sample_ss = u_projection * vec4(sample_vs, 1.0);
    sample_ss.xyz /= sample_ss.w;
    sample_ss.xyz = sample_ss.xyz * 0.5 + 0.5;

    float sample_depth_vs = GetLinearDepthFromVS(texture(ut_position_vs, sample_ss.xy).z);
    if (sample_depth_vs + 0.025 < depth_vs) {
      float range_check = smoothstep(0.0, 1.0, radius / abs(depth_vs - sample_depth_vs));
      occlusion += range_check;
    }
  }
  occlusion = 1.0 - occlusion / kernel_size;
  out_frag_color = vec3(occlusion, occlusion, occlusion);
}
