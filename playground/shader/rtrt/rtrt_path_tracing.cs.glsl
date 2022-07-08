#include "renderer/shader/version.glsl"

#include "renderer/shader/camera.glsl"
#include "renderer/shader/color.glsl"
#include "renderer/shader/fr/BRDF.glsl"
#include "renderer/shader/path_tracing.glsl"
#include "renderer/shader/primitive.glsl"
#include "renderer/shader/random.glsl"
#include "renderer/shader/sample.glsl"
#include "renderer/shader/scene.glsl"
#include "renderer/shader/transform.glsl"

layout (local_size_x = 32, local_size_y = 32) in;
layout (rgba32f, binding = 0) uniform image2D rasterized_position_ws;
layout (rgba32f, binding = 1) uniform image2D rasterized_surface_normal_ws;
layout (r32f, binding = 2) uniform image2D rasterized_primitive_index;
layout (rgba32f, binding = 3) uniform image2D current_ping;

uniform vec2 resolution;
uniform int frame_num;
uniform Camera camera;

// https://www.bilibili.com/video/BV1X7411F744?p=16 0:58:08
void main() {
  InitRNG(gl_GlobalInvocationID.xy, frame_num);

  vec3 near_pos_ss = vec3(gl_GlobalInvocationID.xy / resolution, 0.0);
  vec3 near_pos_ws = PositionSS2WS(camera.view, camera.project, near_pos_ss);
  vec3 Li = normalize(near_pos_ws - camera.pos_ws);
  
  vec3 position_ws = imageLoad(rasterized_position_ws, ivec2(gl_GlobalInvocationID.xy)).xyz;
  vec3 surface_normal_ws = imageLoad(rasterized_surface_normal_ws, ivec2(gl_GlobalInvocationID.xy)).xyz;
  vec3 Lo = SampleUnitHemisphereDir(surface_normal_ws);
  int primitive_index = int(imageLoad(rasterized_primitive_index, ivec2(gl_GlobalInvocationID.xy)).x);

  FromRasterization from_rasterization;
  from_rasterization.Li = Li;
  from_rasterization.Lo = Lo;
  from_rasterization.position = position_ws;
  from_rasterization.normal = surface_normal_ws;
  from_rasterization.primitive_index = primitive_index;
  
  FromCamera from_camera;
  from_camera.ray = Ray(camera.pos_ws, Li);
  from_camera.normal = camera.front;

  // vec4 color = path_tracing_from_camera(from_camera, false);
  vec4 color = path_tracing_from_rasterization(from_rasterization, false);
  imageStore(current_ping, ivec2(gl_GlobalInvocationID.xy), color);
}