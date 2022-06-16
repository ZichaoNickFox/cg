#include "engine/shader/version.glsl"

#include "engine/shader/camera.glsl"
#include "engine/shader/convert.glsl"
#include "engine/shader/geometry.glsl"

uniform vec2 screen_size;
uniform Camera camera;
uniform Sphere spheres[10];
uniform mat4 view;
uniform mat4 project;

layout (local_size_x = 32, local_size_y = 32) in;
layout (rgba32f, binding = 0) uniform image2D canvas;
layout (std430, binding = 0) buffer LightPath { vec4 light_path[20]; };

struct RayTracingResult {
  struct Sphere sphere;
  struct RaySphereResult hit_info;
};

RayTracingResult RayTracing(Sphere spheres[10], Ray ray, float limit) {
  RayTracingResult res;
  float t_min = 99999;
  for (int i = 0; i < 10; ++i) {
    Sphere sphere = spheres[i];
    RaySphereResult hit_result = RaySphere(sphere, ray, limit);
    if (hit_result.hitted && hit_result.dist <= camera.far && t_min >= hit_result.dist) {
      t_min = hit_result.dist;
      res = RayTracingResult(sphere, hit_result);
    }
  }
  return res;
}

void main() {
  vec3 near_pos_ss = vec3(gl_GlobalInvocationID.xy / screen_size, 0.0);
  vec3 near_pos_ws = PositionSS2WS(near_pos_ss, view, project);
  vec3 dir = normalize(near_pos_ws - camera.pos_ws);
  const vec4 clear_color = vec4(0, 0, 1, 1);

  bool is_debug_frag = length(gl_GlobalInvocationID.xy / screen_size- vec2(0.5, 0.5)) < 0.00001;

  const int depth_num = 4;
  vec4 weights[depth_num] = vec4[depth_num](vec4(0.4, 0.4, 0.4, 1.0),
                                            vec4(0.3, 0.3, 0.3, 1.0),
                                            vec4(0.2, 0.2, 0.2, 1.0),
                                            vec4(0.1, 0.1, 0.1, 1.0));
  vec4 colors[depth_num] = vec4[depth_num](vec4(0, 0, 0, 1),
                                           vec4(0, 0, 0, 1),
                                           vec4(0, 0, 0, 1),
                                           vec4(0, 0, 0, 1));
  int depth_iter = 0;
  vec4 obj_color[depth_num];

  Sphere light = Sphere(11, vec3(0, 2.0, 1.1), vec4(10, 10, 10, 10), 0);
  const float bias = 0.0001;
  vec3 ray_dir = dir;
  vec3 ray_from = camera.pos_ws + bias * ray_dir;
  if (is_debug_frag) {
    ray_dir =vec3(-0.137081, -0.281153, -0.949822);
    ray_from = vec3(0.000000, 1.000000, 5.000000);
  }

  while (depth_iter < depth_num) {
    if (is_debug_frag) {
      light_path[depth_iter] = vec4(ray_from, 1.0);
    }
    Ray color_ray = Ray(ray_from, ray_dir);
    RayTracingResult color_result = RayTracing(spheres, color_ray, 50);
    if (!color_result.hit_info.hitted) {
      break;
    }
    vec3 L = vec3(0, 0, 0);
    vec3 N = vec3(0, 0, 0);
    L = normalize(light.center_pos - color_result.hit_info.pos);
    N = normalize(color_result.hit_info.pos - color_result.sphere.center_pos);
    colors[depth_iter] = max(dot(L, N), 0) * color_result.sphere.color;
    colors[depth_iter].w = 1.0;

    // shadow
    vec3 origin = color_result.hit_info.pos + color_result.hit_info.normal * bias;
    vec3 light_dir = normalize(light.center_pos - color_result.hit_info.pos);
    float light_dist = length(light.center_pos - color_result.hit_info.pos);
    Ray shadow_ray = Ray(origin, light_dir);
    RayTracingResult shadow_result = RayTracing(spheres, shadow_ray, 50);
    if (shadow_result.hit_info.hitted && light_dist >= shadow_result.hit_info.dist) {
      colors[depth_iter] = colors[depth_iter] / 2;
      colors[depth_iter].w = 1.0;
    }

    // glass
    if (color_result.sphere.id == 9){
      if (dot(ray_dir, N) < 0) {
        float ratio = 1 / 1.52;
        ray_dir = refract(ray_dir, N, ratio);
      } else {
        float ratio = 1.52 / 1;
        ray_dir = refract(ray_dir, -N, ratio);
      }
    } else {
      ray_dir = reflect(ray_dir, N);
    }
    ray_from = color_result.hit_info.pos + bias * ray_dir;

    depth_iter = depth_iter + 1;
  }

  vec4 color = vec4(0, 0, 0, 1);
  for (int i = 0; i < 4; ++i) {
    color = color + colors[i] * weights[i];
    color.w = 1.0;
  }

  // Lambert model
  imageStore(canvas, ivec2(gl_GlobalInvocationID.xy), color);
}