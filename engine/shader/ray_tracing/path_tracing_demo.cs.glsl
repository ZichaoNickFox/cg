#include "engine/shader/version.glsl"

#include "engine/shader/bvh.glsl"
#include "engine/shader/camera.glsl"
#include "engine/shader/convert.glsl"
#include "engine/shader/geometry.glsl"
#include "engine/shader/pbr/pbr_BRDF.glsl"
#include "engine/shader/random.glsl"
#include "engine/shader/sample.glsl"
#include "engine/shader/uniform.glsl"

const float pi = 3.1415926;
const float bias = 0.0001;

// https://www.bilibili.com/video/BV1X7411F744?p=16 0:58:08
vec4 path_tracing(Ray ray, vec4 color) {
  Ray ray_iter = ray;
  vec4 radiance = vec4(1, 1, 1, 1);

//  bool is_debug_frag = length(gl_GlobalInvocationID.xy / screen_size - vec2(0.5, 0.5)) < 0.00001;
  bool is_debug_frag = false;
  if (is_debug_frag) {
    ray_iter.origin = vec3(0.000000, 1.000000, 5.000000);
    ray_iter.dir = vec3(0.251670, -0.375554, -0.891976);
  }

  int count = 0;
  while(true) {
    if (is_debug_frag) {
      light_path[count] = vec4(ray_iter.origin, 1.0);
    }
    if (count >= 10) {
      break;
    }
    count += 1;

    const float P_RR = 0.95;
    if (Random() > P_RR) {
      break;
    }

    Sphere sphere;
    RaySphereResult result;
    float min_dist = 9999;
    for (int i = 0; i < 10; ++i) {
      RaySphereResult temp_result = RaySphere(spheres[i], ray_iter, 50);
      if (temp_result.hitted && temp_result.dist < min_dist) {
        sphere = spheres[i];
        result = temp_result;
        min_dist = temp_result.dist;
      }
    }

    const float pdf = 1 / (2 * pi);
    if (sphere.id == 1) {
      // light
      color = radiance * sphere.color / P_RR;
      if (is_debug_frag) {
        light_path[count] = vec4(result.pos, 1.0);
      }
    } else {
      // right big metal ball
      vec3 dir_ws = SampleUnitHemisphereDir(result.normal);
      float f_r_specular = BRDF_specular(-ray_iter.dir, dir_ws, result.normal, 0.5);
      vec4 f_r_deffuse = BRDF_diffuse(sphere.color);
      vec4 f_r = 0.0 * f_r_specular + 1.0 * f_r_deffuse;
      float cosine = max(dot(result.normal, dir_ws), 0.0);
      radiance = radiance * f_r * cosine / pdf / P_RR;
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
  vec4 acc_color = color;
  const int sample_num = 3;
  for (int i = 0; i < sample_num; ++i) {
    acc_color += path_tracing(ray, color);
  }
  color = acc_color / (sample_num + 1);
  imageStore(canvas, ivec2(gl_GlobalInvocationID.xy), color);
}