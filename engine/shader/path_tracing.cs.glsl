#include "engine/shader/version.glsl"

#include "engine/shader/camera.glsl"
#include "engine/shader/convert.glsl"
#include "engine/shader/geometry.glsl"
#include "engine/shader/pbr/pbr_BRDF.glsl"
#include "engine/shader/random.glsl"
#include "engine/shader/sample.glsl"

uniform vec2 screen_size;
uniform Camera camera;
uniform Sphere spheres[10];
uniform mat4 view;
uniform mat4 project;

layout (local_size_x = 32, local_size_y = 32) in;
layout (rgba32f, binding = 0) uniform image2D canvas;
layout (std430, binding = 0) buffer LightPath { vec4 light_path[20]; };

const float pi = 3.1415926;
const float bias = 0.0001;

// https://www.bilibili.com/video/BV1X7411F744?p=16 0:58:08
vec4 path_tracing(Sphere spheres2[10], Ray ray, vec4 color) {
  Ray ray_iter = ray;
  vec4 color_iter = vec4(1, 1, 1, 1);

  bool is_debug_frag = length(gl_GlobalInvocationID.xy / screen_size - vec2(0.5, 0.5)) < 0.00001;
  if (is_debug_frag) {
    ray_iter.origin = vec3(0.000000, 1.000000, 5.000000);
    ray_iter.dir = vec3(0.251670, -0.375554, -0.891976);
  }

  int count = 0;
  while(true) {
    if (count >= 20) {
      break;
    }

    if (is_debug_frag) {
      light_path[count] = vec4(ray_iter.origin, 1.0);
    }

    count += 1;

    const float P_RR = 0.9;
    if (Random() > P_RR) {
      break;
    }

    Sphere sphere;
    RaySphereResult result;
    float min_dist = 9999;
    for (int i = 0; i < 10; ++i) {
      RaySphereResult temp_result = ray_sphere(spheres[i], ray_iter, 50);
      if (temp_result.hitted && temp_result.dist < min_dist) {
        sphere = spheres[i];
        result = temp_result;
        min_dist = temp_result.dist;
      }
    }

    if (sphere.id == 1) {
      // light
      color = color_iter * sphere.color * max(dot(result.normal, -ray_iter.dir), 0.0) / (pi * 2) / P_RR;
      if (is_debug_frag) {
        light_path[count] = vec4(result.pos, 1.0);
      }
//    } else if (sphere.id == 9) {
      // glass
//      vec3 dir_ws = SampleSphereRandomDirction(result.normal);
//      if (dot(dir_ws, result.normal) < 0) {
        
//      }
    } else if (sphere.id == 8) {
      // right big metal ball
      vec3 dir_ws = SampleUnitHemisphereDir(result.normal);
      float f_r = BRDF(-ray_iter.dir, dir_ws, result.normal, 0.1);
//      float f_r = 1;
      color_iter = color_iter * sphere.color * max(dot(result.normal, -ray_iter.dir), 0.0) * f_r / /*(pi * 2) */ P_RR;
      ray_iter = Ray(result.pos + bias * dir_ws, dir_ws);
    } else {
      // other
      vec3 dir_ws = SampleUnitHemisphereDir(result.normal);
      float f_r = BRDF(-ray_iter.dir, dir_ws, result.normal, 0.5);
//      float f_r = 1.0;
      color_iter = color_iter * sphere.color * max(dot(result.normal, -ray_iter.dir), 0.0) * f_r / /*(pi * 2) */ P_RR;
      ray_iter = Ray(result.pos + bias * dir_ws, dir_ws);
    }
  }
  return color;
}

void main() {
  InitRNG(gl_GlobalInvocationID.xy);

  vec3 near_pos_ss = vec3(gl_GlobalInvocationID.xy / screen_size, 0.0);
  vec3 near_pos_ws = PositionSS2WS(near_pos_ss, view, project);
  vec3 camera_dir_ws = normalize(near_pos_ws - camera.pos_ws);
  
  vec4 color = imageLoad(canvas, ivec2(gl_GlobalInvocationID.xy));
  Ray ray = Ray(camera.pos_ws + bias * camera_dir_ws, camera_dir_ws);
  color = path_tracing(spheres, ray, color); 

  imageStore(canvas, ivec2(gl_GlobalInvocationID.xy), color);
}