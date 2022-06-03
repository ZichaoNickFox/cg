uniform vec2 screen_size;
uniform CameraGeometry camera_geometry;
uniform SphereGeometry sphere_geometries[10];
uniform mat4 view;
uniform mat4 project;

layout (local_size_x = 32, local_size_y = 32) in;
layout (rgba32f, binding = 0) uniform image2D texture_output;

void main() {
  vec3 near_pos_ss = vec3(gl_GlobalInvocationID.xy / screen_size, 0.0);
  vec3 near_pos_ws = PositionSS2WS(near_pos_ss, view, project);

  vec3 dir = normalize(near_pos_ws - camera_geometry.pos_ws);

  vec3 light_center = vec3(0, 2.0, 1);

  float dist;
  vec3 N = vec3(0, 0, 0);
  vec3 hit_pos_ws = vec3(0, 0, 0);
  vec3 L = vec3(0, 0, 0);

  vec4 color = vec4(0, 0, 1, 1);
  float t_min = 99999;
  for (int i = 0; i < 10; ++i) {
    SphereGeometry sphere = sphere_geometries[i];
    HitSphereResult hit_result = hit_sphere(sphere, camera_geometry.pos_ws, dir);
    if (hit_result.hitted && hit_result.dist <= camera_geometry.far && t_min > hit_result.dist) {
      t_min = hit_result.dist;
      color = sphere.color;

      // interact with light
      L = normalize(light_center - hit_result.hit_pos_ws);
      N = normalize(hit_result.hit_pos_ws - sphere.center_pos_ws);
      color = max(dot(L, N), 0) * color;
    }
  }

  imageStore(texture_output, ivec2(gl_GlobalInvocationID.xy), color);
}