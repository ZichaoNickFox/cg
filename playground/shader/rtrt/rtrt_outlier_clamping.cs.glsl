#include "renderer/shader/version.glsl"

#include "renderer/shader/sample.glsl"

layout (local_size_x = 32, local_size_y = 32) in;
layout (rgba32f, binding = 0) uniform image2D texture_in;
layout (rgba32f, binding = 1) uniform image2D texture_out;
uniform vec2 resolution;

void main() {
  InitRNG(gl_GlobalInvocationID.xy);

  vec4 color = imageLoad(texture_in, ivec2(gl_GlobalInvocationID.xy));

  // https://www.bilibili.com/video/BV1YK4y1T7yY?p=13 0:20:37
  int radius = 3;
  int row_begin = max(int(gl_GlobalInvocationID.y) - radius, 0);
  int row_end = min(int(gl_GlobalInvocationID.y) + radius + 1, int(resolution.y));
  int col_begin = max(int(gl_GlobalInvocationID.x) - radius, 0);
  int col_end = min(int(gl_GlobalInvocationID.x) + radius + 1, int(resolution.x));
  vec4 mean = vec4(0, 0, 0, 0);
  int n = 0;
  for (int r = row_begin; r < row_end; ++r) {
    for (int c = col_begin; c < col_end; ++c) {
      vec4 color = imageLoad(texture_in, ivec2(c, r));
      mean += color;
      n++;
    }
  }
  mean /= n;
  vec4 variance = vec4(0, 0, 0, 0);
  for (int r = row_begin; r < row_end; ++r) {
    for (int c = col_begin; c < col_end; ++c) {
      vec4 color = imageLoad(texture_in, ivec2(c, r));
      variance += (mean - color) * (mean - color);
    }
  }
  vec4 standard_deviation = vec4(0, 0, 0, 0);
  variance /= n;
  standard_deviation = sqrt(variance);
  color = clamp(color, mean - standard_deviation * 3, mean + standard_deviation * 3);

  imageStore(texture_out, ivec2(gl_GlobalInvocationID.xy), color);
}