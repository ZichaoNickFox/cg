uniform vec2 screen_size;
uniform Camera camera;
uniform Sphere spheres[10];
uniform mat4 view;
uniform mat4 project;

layout (local_size_x = 32, local_size_y = 32) in;
layout (rgba32f, binding = 0) uniform image2D texture_output;

struct RayTracingResult {
  struct Sphere sphere;
  struct RaySphereResult hit_info;
};

// https://www.bilibili.com/video/BV1X7411F744?p=16 0:58:08
RayTracingResult path_tracing(Sphere spheres[10], Ray ray) {
  const float P_RR = 0.8;
  if (random() > P_RR) {
    return RayTracingResult();
  }

  
}

void main() {
  vec3 near_pos_ss = vec3(gl_GlobalInvocationID.xy / screen_size, 0.0);
  vec3 near_pos_ws = PositionSS2WS(near_pos_ss, view, project);
  vec3 camera_dir_ws = normalize(near_pos_ws - camera.pos_ws);
  const vec4 clear_color = vec4(0, 0, 1, 1);
  
  

  // Lambert model
  imageStore(texture_output, ivec2(gl_GlobalInvocationID.xy), color);
}