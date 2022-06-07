uniform vec2 screen_size;
uniform Camera camera;
uniform Sphere spheres[10];
uniform mat4 view;
uniform mat4 project;

layout (local_size_x = 32, local_size_y = 32) in;
layout (rgba32f, binding = 0) uniform image2D texture_output;
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
    RaySphereResult hit_result = ray_sphere(sphere, ray, limit);
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

  const int depth_num = 4;
  /*
  vec4 weights[depth_num] = vec4[depth_num](vec4(0.3, 0.3, 0.3, 1.0),
                                            vec4(0.3, 0.3, 0.3, 1.0),
                                            vec4(0.3, 0.3, 0.3, 1.0),
                                            vec4(0.3, 0.3, 0.3, 1.0));
                                            */
  vec4 weights[depth_num] = vec4[depth_num](vec4(1,1,1,1),
                                            vec4(1,1,1,1),
                                            vec4(1,1,1,1),
                                            vec4(1,1,1,1));
  vec4 colors[depth_num] = vec4[depth_num](vec4(0, 0, 0, 1),
                                           vec4(0, 0, 0, 1),
                                           vec4(0, 0, 0, 1),
                                           vec4(0, 0, 0, 1));
  int depth_iter = 0;
  vec4 obj_color[depth_num];

  Sphere light = Sphere(vec3(0, 2.0, 1.1), vec4(10, 10, 10, 10), 0);
  const float bias = 0.0001;
  vec3 ray_dir = dir;
  vec3 ray_from = camera.pos_ws + bias * ray_dir;
  while (depth_iter < depth_num) {
//    light_path[depth_iter] = ray_from;
    Ray color_ray = Ray(ray_from, ray_dir);
    RayTracingResult color_result = RayTracing(spheres, color_ray, 50);
    if (!color_result.hit_info.hitted) {
      break;
    }
    vec3 L = vec3(0, 0, 0);
    vec3 N = vec3(0, 0, 0);
    L = normalize(light.center_pos - color_result.hit_info.pos);
    N = normalize(color_result.hit_info.pos - color_result.sphere.center_pos);
//    colors[depth_iter] = max(dot(L, N), 0) * color_result.sphere.color;
    colors[depth_iter] = 0.5 * color_result.sphere.color;

    if (dot(ray_dir, N) < 0) {
      float ratio = 1 / 1.52;
      ray_dir = refract(ray_dir, N, ratio);
    } else {
      float ratio = 1.52 / 1;
      ray_dir = refract(ray_dir, N, ratio);
    }
    ray_from = color_result.hit_info.pos + bias * ray_dir;

    /*
    // shadow
    vec3 origin = color_result.hit_info.pos + color_result.hit_info.normal * bias;
    vec3 light_dir = normalize(light.center_pos - color_result.hit_info.pos);
    float light_dist = length(light.center_pos - color_result.hit_info.pos);
    Ray shadow_ray = Ray(origin, light_dir);
    RayTracingResult shadow_result = RayTracing(spheres, shadow_ray, 50);
    if (shadow_result.hit_info.hitted && light_dist >= shadow_result.hit_info.dist) {
      colors[depth_iter] = colors[depth_iter] / 2;
    }
    */

//    ray_dir = reflect(ray_dir, N);
//    ray_from = color_result.hit_info.pos + bias * ray_dir;
    depth_iter = depth_iter + 1;
  }

  vec4 color = vec4(0, 0, 0, 1);
  for (int i = 2; i < 3; ++i) {
    color = color + colors[i] * weights[i];
    color = colors[i];
  }

  if (length(gl_GlobalInvocationID.xy / screen_size- vec2(0.5, 0.5)) < 0.00001) {
  for (int i =0; i< 20; ++i) {
    light_path[i] = vec4(i, i, i, 1);
  }
  }

  // Lambert model
  imageStore(texture_output, ivec2(gl_GlobalInvocationID.xy), color);
}