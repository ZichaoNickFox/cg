#include "renderer/shader/version.glsl"

#include "renderer/shader/camera.glsl"
#include "renderer/shader/color.glsl"
#include "renderer/shader/sample.glsl"
#include "renderer/shader/transform.glsl"

layout (local_size_x = 32, local_size_y = 32) in;
layout (rgba32f, binding = 0) uniform image2D texture_position_ws;
layout (rgba32f, binding = 1) uniform image2D current_ping;
layout (rgba32f, binding = 2) uniform image2D last_ping;
layout (rgba32f, binding = 3) uniform image2D last_pong;
uniform vec2 resolution;
uniform Camera camera_1;
uniform int frame_num;

// https://www.bilibili.com/video/BV1YK4y1T7yY?p=12 0:58:42
void main() {
  InitRNG(gl_GlobalInvocationID.xy, frame_num);

  vec4 color = vec4(0,0,0,1);
  vec4 color_1 = vec4(0, 0, 0, 1);
  vec4 position_ws = imageLoad(texture_position_ws, ivec2(gl_GlobalInvocationID.xy));
  color = imageLoad(current_ping, ivec2(gl_GlobalInvocationID.xy));

  // Static Scene : Last position_ws == This position_ws
  vec4 position_ws_1 = position_ws;
  vec4 position_ss_1 = PositionWS2SS(camera_1.view, camera_1.project, position_ws_1.xyz);
  if (all(equal(position_ws_1, vec4(0, 0, 0, 1)))) {
    color = vec4(0, 0, 0, 1);
  } else if (all(greaterThanEqual(position_ss_1.xy, vec2(0, 0))) && all(lessThanEqual(position_ss_1.xy, vec2(1, 1)))) {
    color_1 = imageLoad(last_ping, ivec2(position_ss_1.xy * resolution));
    if (color != vec4(0, 0, 0, 1)) {
      color = color;
    } else {
      color = color_1;
    }
  }

  imageStore(last_pong, ivec2(gl_GlobalInvocationID.xy), color);
}