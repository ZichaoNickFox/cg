#include "renderer/shader/version.glsl"

#include "renderer/shader/bvh.glsl"
#include "renderer/shader/camera.glsl"
#include "renderer/shader/compute_shader.glsl"
#include "renderer/shader/color.glsl"
#include "renderer/shader/convert.glsl"
#include "renderer/shader/geometry.glsl"
#include "renderer/shader/pbr/pbr_BRDF.glsl"
#include "renderer/shader/random.glsl"
#include "renderer/shader/sample.glsl"

uniform vec2 screen_size;
uniform Camera camera;
uniform mat4 view;
uniform mat4 project;

layout (rgba32f, binding = 0) uniform image2D canvas;

const float pi = 3.1415926;
const float bias = 0.0001;

// https://www.bilibili.com/video/BV1X7411F744?p=16 0:58:08
vec4 path_tracing(Ray ray, vec4 color) {
  Ray ray_iter = ray;
  vec4 radiance = vec4(1, 1, 1, 1);

  int count = 10;
  while(bool(count--)) {
    const float P_RR = 0.95;
    if (Random() > P_RR) {
      break;
    }

    RayBVHResult result = RayBVH(ray_iter);
    if (result.hitted == false) {
      return color;
    }
    Material material = material_repo[result.material_index];
    vec3 N = result.normal;

    const float pdf = 1 / (2 * pi);
    if (MaterialEmission(material) != vec4(0, 0, 0, 1)) {
      // light
      float cosine = max(dot(-ray_iter.dir, N), 0);
      color = radiance * MaterialDiffuse(material) * cosine / pdf / P_RR;
      break;
    } else {
      vec3 dir_ws = SampleUnitHemisphereDir(N);
      float f_r_specular = BRDF_specular(-ray_iter.dir, dir_ws, result.normal, 0.5);
      vec4 f_r_diffuse = BRDF_diffuse(MaterialDiffuse(material));
      vec4 f_r = 0.0 * f_r_specular + 1.0 * f_r_diffuse;
      float cosine = max(dot(dir_ws, N), 0.0);
      radiance = radiance * f_r * cosine / pdf / P_RR;
      ray_iter = Ray(result.pos + bias * dir_ws, dir_ws);
    }
  }
  return color;
}

void main() {
  InitRNG(gl_GlobalInvocationID.xy);

  vec3 near_pos_ss = vec3(gl_GlobalInvocationID.xy / screen_size, 0.0);
  vec3 near_pos_ws = PositionSS2WS(camera.view, camera.project, near_pos_ss);
  vec3 camera_dir_ws = normalize(near_pos_ws - camera.pos_ws);
  
  vec4 color = imageLoad(canvas, ivec2(gl_GlobalInvocationID.xy));
  Ray ray = Ray(camera.pos_ws, camera_dir_ws);
  color += path_tracing(ray, color);
  color /= 2;
  imageStore(canvas, ivec2(gl_GlobalInvocationID.xy), color);
}