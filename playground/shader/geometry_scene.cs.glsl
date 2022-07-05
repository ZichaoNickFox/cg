#include "renderer/shader/version.glsl"

#include "renderer/shader/camera.glsl"
#include "renderer/shader/color.glsl"
#include "renderer/shader/compute_shader.glsl"
#include "renderer/shader/definition.glsl"
#include "renderer/shader/geometry.glsl"
#include "renderer/shader/transform.glsl"

layout (rgba32f, binding = 0) uniform image2D canvas;

uniform int triangle_num;
layout (std430, binding = SSBO_USER_0) buffer Triangles {
  Triangle triangles[];
};

uniform int aabb_num;
layout (std430, binding = SSBO_USER_1) buffer AABBs {
  AABB aabbs[];
};

uniform vec2 screen_size;
uniform Camera camera;

void main() {
  vec3 near_pos_ss = vec3(gl_GlobalInvocationID.xy / screen_size, 0.0);
  vec3 near_pos_ws = PositionSS2WS(camera.view, camera.project, near_pos_ss);
  vec3 dir = normalize(near_pos_ws - camera.pos_ws);
  const vec4 clear_color = vec4(0, 0, 1, 1);

  Ray ray = Ray(near_pos_ws, dir);

  vec4 color = kClearColor;
  for (int i = 0; i < triangle_num; ++i) {
    RayTriangleResult result = RayTriangle(ray, triangles[i]);
    if (result.hitted) {
      color = kRed;
    }
  }
  for (int i = 0; i < aabb_num; ++i) {
    RayAABBResult result = RayAABB(ray, aabbs[i]);
    if (result.hitted) {
      color = kGreen;
    }
  }

  imageStore(canvas, ivec2(gl_GlobalInvocationID.xy), color);
}