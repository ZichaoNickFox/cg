uniform vec2 screen_size;
uniform CameraGeometry camera_geometry;
uniform SphereGeometry sphere_geometries[10];
uniform mat4 view;
uniform mat4 project;

layout (local_size_x = 32, local_size_y = 32) in;
layout (rgba32f, binding = 0) uniform image2D texture_output;

struct RayTracingResult {
  struct SphereGeometry sphere;
  struct HitSphereResult hit_info;
};

RayTracingResult RayTracing(SphereGeometry spheres[10], vec3 from_ws, vec3 dir_ws) {
  RayTracingResult res;
  float t_min = 99999;
  for (int i = 0; i < 10; ++i) {
    SphereGeometry sphere = spheres[i];
    HitSphereResult hit_result = hit_sphere(sphere, from_ws, dir_ws);
    if (hit_result.hitted && hit_result.dist <= camera_geometry.far && t_min >= hit_result.dist) {
      t_min = hit_result.dist;
      res = RayTracingResult(sphere, hit_result);
    }
  }
  return res;
}

void main() {
  vec3 near_pos_ss = vec3(gl_GlobalInvocationID.xy / screen_size, 0.0);
  vec3 near_pos_ws = PositionSS2WS(near_pos_ss, view, project);
  vec3 dir = normalize(near_pos_ws - camera_geometry.pos_ws);

  SphereGeometry light = SphereGeometry(vec3(0, 2.0, 1), vec4(10, 10, 10, 10), 1);

  vec4 color = vec4(0, 0, 1, 1);
  RayTracingResult color_result = RayTracing(sphere_geometries, camera_geometry.pos_ws, dir);
  if (color_result.hit_info.hitted) {
    vec3 L = normalize(light.center_pos_ws - color_result.hit_info.pos_ws);
    vec3 N = normalize(color_result.hit_info.pos_ws - color_result.sphere.center_pos_ws);
    color = max(dot(L, N), 0) * color_result.sphere.color;
  }

  const float bias = 0.00001;
  vec3 origin = color_result.hit_info.pos_ws + color_result.hit_info.normal_ws * bias;
  vec3 light_dir = normalize(light.center_pos_ws - color_result.hit_info.pos_ws);
  float light_dist = length(light.center_pos_ws - color_result.hit_info.pos_ws);
  RayTracingResult shadow_result = RayTracing(sphere_geometries, origin, light_dir);
  if (shadow_result.hit_info.hitted && light_dist >= shadow_result.hit_info.dist) {
    color = color / 2;
  }

  // Lambert model
  imageStore(texture_output, ivec2(gl_GlobalInvocationID.xy), color);
}