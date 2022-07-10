#include "renderer/shader/version.glsl"

#include "renderer/shader/camera.glsl"
#include "renderer/shader/color.glsl"
#include "renderer/shader/depth.glsl"
#include "renderer/shader/random.glsl"
#include "renderer/shader/sample.glsl"
#include "renderer/shader/transform.glsl"

layout (local_size_x = 32, local_size_y = 32) in;

layout (rgba32f, binding = 0) uniform image2D texture_position_vs;
layout (rgba32f, binding = 1) uniform image2D texture_normal_vs;
layout (binding = 2) uniform sampler2D texture_depth;
layout (rgba32f, binding = 3) uniform image2D texture_out;

uniform vec2 resolution;
uniform int frame_num;
uniform mat4 model;
uniform Camera camera;

void main() {
  InitRNG(gl_GlobalInvocationID.xy, frame_num);

  vec4 color = vec4(1, 1, 1, 1);
  vec3 position_vs = imageLoad(texture_position_vs, ivec2(gl_GlobalInvocationID.xy)).xyz;
  vec3 normal_vs = imageLoad(texture_normal_vs, ivec2(gl_GlobalInvocationID.xy)).xyz;
  float occlusion = 0.0;
  int sample_num = 5;
  for (int i = 0; i < sample_num; ++i) {
    vec3 sample_vs = position_vs + SampleUnitHemisphereDir(normal_vs) * Random() * 0.05;
    vec4 sample_ss = PositionVS2SS(camera.project, sample_vs);
    if (sample_ss.x < 0 || sample_ss.x > 1 || sample_ss.y < 0 || sample_ss.y > 1) { continue; }
    float sample_depth_ss = texture(texture_depth, sample_ss.xy).x;
    float sample_depth_vs = DepthSS2VS(sample_depth_ss, camera.near, camera.far);
    float max_range = 0.2;
    const float kBias = 0.02;
    if (-position_vs.z > -sample_depth_vs + kBias && abs(position_vs.z - sample_depth_vs) < max_range) {
      occlusion += 1;
    }
  }
  occlusion = 1 - occlusion / sample_num;
  color = color * occlusion;

  imageStore(texture_out, ivec2(gl_GlobalInvocationID.xy), color);
}
