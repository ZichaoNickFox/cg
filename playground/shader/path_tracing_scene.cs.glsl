#include "renderer/shader/version.glsl"

#include "renderer/shader/bvh.glsl"
#include "renderer/shader/camera.glsl"
#include "renderer/shader/color.glsl"
#include "renderer/shader/geometry.glsl"
#include "renderer/shader/path_tracing.glsl"
#include "renderer/shader/random.glsl"
#include "renderer/shader/sample.glsl"
#include "renderer/shader/scene.glsl"
#include "renderer/shader/transform.glsl"

layout (local_size_x = 32, local_size_y = 32) in;
layout (rgba32f, binding = 0) uniform image2D texture_in_out;

uniform vec2 resolution;
uniform Camera camera;
uniform bool dirty;

void main() {
  InitRNG(gl_GlobalInvocationID.xy);

  vec3 near_pos_ss = vec3(gl_GlobalInvocationID.xy / resolution, 0.0);
  vec3 near_pos_ws = PositionSS2WS(camera.view, camera.project, near_pos_ss);
  vec3 camera_dir_ws = normalize(near_pos_ws - camera.pos_ws);
  
  vec4 color = imageLoad(texture_in_out, ivec2(gl_GlobalInvocationID.xy));
  if (dirty) {
    color = kBlack;
  } else {
    Ray ray = Ray(camera.pos_ws, camera_dir_ws);
    vec4 path_tracing_result = path_tracing(ray, vec4(1, 1, 1, 1), 5, 0.9);
    if (color == vec4(0, 0, 0, 1)) {
      color = path_tracing_result;
    } else if (path_tracing_result == vec4(0, 0, 0, 1)) {
      color = color;
    } else {
      color = (color + path_tracing_result) / 2;
    }
  }
  imageStore(texture_in_out, ivec2(gl_GlobalInvocationID.xy), color);
}