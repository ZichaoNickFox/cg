uniform vec2 screen_size;
uniform Camera camera;
uniform Sphere spheres[10];
uniform mat4 view;
uniform mat4 project;

layout (local_size_x = 32, local_size_y = 32) in;
layout (rgba32f, binding = 0) uniform image2D texture_output;
layout (std430, binding = 0) buffer LightPath { vec4 light_path[20]; };

const float pi = 3.1415926;
const float bias = 0.0001;

// https://www.bilibili.com/video/BV1X7411F744?p=16 0:58:08
vec4 path_tracing(Sphere spheres2[10], Ray ray) {
  vec4 res = vec4(0, 0, 0, 1);
  Ray ray_iter = ray;
  vec4 color_iter = vec4(1, 1, 1, 1);

//  bool is_debug_frag = length(gl_GlobalInvocationID.xy / screen_size - vec2(0.5, 0.5)) < 0.00001;
  bool is_debug_frag = false;
  if (is_debug_frag) {
    ray_iter.origin = vec3(0.000000, 1.000000, 5.000000);
    ray_iter.dir = vec3(-0.023336, -0.358501, -0.933238);
  }

  int count = 0;
  while(true) {
    if (count >= 20) {
      res = vec4(0, 0, 0, 1);
      break;
    }

    if (is_debug_frag) {
      light_path[count] = vec4(ray_iter.origin, 1.0);
    }

    count += 1;

    const float P_RR = 0.8;
    if (random_cs(count) > P_RR) {
      res = vec4(0, 0, 0, 1);
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

    // light
    if (sphere.id == 1) {
      res = sphere.color * dot(result.normal, -ray_iter.dir) / (pi * 2) / P_RR * color_iter;
      if (is_debug_frag) {
        light_path[count] = vec4(result.pos, 1.0);
      }
      break;
    } else {
      // hemisphere sphere tangent space 
      vec3 x_ts_base = cross(vec3(0, 1, 0), result.normal);
      vec3 z_ts_base = cross(x_ts_base, result.normal);
      vec3 y_ts_base = result.normal;
      float x_ts = random_cs(1) * 2 - 1;
      float z_ts = random_cs(2) * 2 - 1;
      float y_ts = random_cs(3);
      vec3 dir_ws = mat3(x_ts_base, y_ts_base, z_ts_base) * vec3(x_ts, y_ts, z_ts);
      dir_ws = normalize(dir_ws);
      ray_iter = Ray(result.pos + bias * dir_ws, dir_ws);
      color_iter = color_iter * sphere.color * dot(result.normal, dir_ws) / (pi * 2) / P_RR;
    }
  }
  return res;
}

void main() {
  vec3 near_pos_ss = vec3(gl_GlobalInvocationID.xy / screen_size, 0.0);
  vec3 near_pos_ws = PositionSS2WS(near_pos_ss, view, project);
  vec3 camera_dir_ws = normalize(near_pos_ws - camera.pos_ws);
  
  Ray ray = Ray(camera.pos_ws + bias * camera_dir_ws, camera_dir_ws);
  vec4 color = path_tracing(spheres, ray); 

  if (color != vec4(0, 0, 0, 1)) {
    imageStore(texture_output, ivec2(gl_GlobalInvocationID.xy), color);
  }
}