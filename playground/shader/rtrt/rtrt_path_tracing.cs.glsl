#include "renderer/shader/version.glsl"

#include "renderer/shader/camera.glsl"
#include "renderer/shader/color.glsl"
#include "renderer/shader/path_tracing.glsl"
#include "renderer/shader/pbr/pbr_BRDF.glsl"
#include "renderer/shader/random.glsl"
#include "renderer/shader/sample.glsl"
#include "renderer/shader/scene.glsl"
#include "renderer/shader/transform.glsl"

layout (local_size_x = 32, local_size_y = 32) in;
layout (rgba32f, binding = 0) uniform image2D in_rasterized_color;
layout (rgba32f, binding = 0) uniform image2D in_rasterized_emission;
layout (rgba32f, binding = 1) uniform image2D in_rasterized_position_ws;
layout (rgba32f, binding = 2) uniform image2D in_rasterized_surface_normal_ws;

uniform vec2 resolution;
uniform int frame_num;
uniform Camera camera;

// https://www.bilibili.com/video/BV1X7411F744?p=16 0:58:08
void main() {
  InitRNG(gl_GlobalInvocationID.xy, frame_num);

  vec3 near_pos_ss = vec3(gl_GlobalInvocationID.xy / resolution, 0.0);
  vec3 near_pos_ws = PositionSS2WS(camera.view, camera.project, near_pos_ss);
  vec3 camera_dir_ws = normalize(near_pos_ws - camera.pos_ws);
  
  vec4 color = imageLoad(in_rasterized_color, ivec2(gl_GlobalInvocationID.xy));
  vec3 position_ws = imageLoad(in_rasterized_position_ws, ivec2(gl_GlobalInvocationID.xy)).xyz;
  vec3 surface_normal_ws = imageLoad(in_rasterized_surface_normal_ws, ivec2(gl_GlobalInvocationID.xy)).xyz;
  vec3 dir_ws = SampleUnitHemisphereDir(surface_normal_ws);
  vec4 emission = imageLoad(in_rasterized_emission, ivec2(gl_GlobalInvocationID.xy));

  bool ray_from_light = emission != vec4(0, 0, 0, 1);
  Ray ray = Ray(position_ws, dir_ws);
  color = path_tracing(ray, color, ray_from_light, 5, 0.9);
  imageStore(in_rasterized_color, ivec2(gl_GlobalInvocationID.xy), color);
}