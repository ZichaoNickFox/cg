#include "renderer/shader/version.glsl"

#include "renderer/shader/sample.glsl"

layout (local_size_x = 32, local_size_y = 32) in;
layout (rgba32f, binding = 0) uniform image2D texture_in;
layout (rgba32f, binding = 1) uniform image2D texture_out;
layout (rgba32f, binding = 2) uniform image2D texture_surface_normal;
uniform vec2 resolution;

// https://www.bilibili.com/video/BV1YK4y1T7yY?p=13
void main() {
  InitRNG(gl_GlobalInvocationID.xy);

  vec4 center_color = imageLoad(texture_in, ivec2(gl_GlobalInvocationID.xy));
  vec4 center_normal = imageLoad(texture_surface_normal, ivec2(gl_GlobalInvocationID.xy));
  vec2 center_pos = gl_GlobalInvocationID.xy;

  int radius = 10;
  int row_begin = max(int(gl_GlobalInvocationID.y) - radius, 0);
  int row_end = min(int(gl_GlobalInvocationID.y) + radius + 1, int(resolution.y));
  int col_begin = max(int(gl_GlobalInvocationID.x) - radius, 0);
  int col_end = min(int(gl_GlobalInvocationID.x) + radius + 1, int(resolution.x));
  float sigma = float(radius) / 3;
  float sum_of_weight = 0;

  float distance_denominator = 1;
  float color_denominator = 2 * sigma * sigma;
  float normal_denominator = 2 * sigma * sigma;

  vec4 sum_of_weighted_value = vec4(0, 0, 0, 0);
  for (int r = row_begin; r < row_end; ++r) {
    for (int c = col_begin; c < col_end; ++c) {
      vec2 pos = vec2(c, r);
      vec4 color = imageLoad(texture_in, ivec2(c, r));
      vec4 normal = imageLoad(texture_surface_normal, ivec2(c, r));

      float distance_item = (pow(center_pos.x - pos.x, 2) + pow(center_pos.y - pos.y, 2)) / distance_denominator;
      float color_item = pow(length(center_color - color), 2) / color_denominator;
      float normal_item = pow(length(center_normal - normal), 2) / normal_denominator;
      float weight = exp(-(distance_item/* + normal_item + color_item*/));

      sum_of_weighted_value += color * weight;
      sum_of_weight += weight;
    }
  }
  center_color = sum_of_weighted_value / sum_of_weight;

  imageStore(texture_out, ivec2(gl_GlobalInvocationID.xy), center_color);
}