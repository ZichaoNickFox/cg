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
    return vec4(result.normal,1.0);

    const float pdf = 1 / (2 * pi);
    if (MaterialEmission(material) != vec4(0, 0, 0, 0)) {
      // light
      color = radiance * MaterialEmission(material) / P_RR;
    } else {
      vec3 dir_ws = SampleUnitHemisphereDir(result.normal);
      // float f_r_specular = BRDF_specular(-ray_iter.dir, dir_ws, result.normal, 0.5);
      // vec4 f_r_deffuse = BRDF_diffuse(MaterialDiffuse(material));
      // vec4 f_r = 0.0 * f_r_specular + 1.0 * f_r_deffuse;
      // float cosine = max(dot(result.normal, dir_ws), 0.0);
      // radiance = radiance * f_r * cosine / pdf / P_RR;
      radiance += MaterialDiffuse(material);
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
  
  // vec4 color = imageLoad(canvas, ivec2(gl_GlobalInvocationID.xy));
  Ray ray = Ray(camera.pos_ws, camera_dir_ws);
  // vec4 acc_color = color;
  // const int sample_num = 3;
  // for (int i = 0; i < sample_num; ++i) {
  //   acc_color += path_tracing(ray, color);
  // }
  // color = acc_color / (sample_num + 1);
  vec4 color = kClearColor;
  color = path_tracing(ray, color);

  imageStore(canvas, ivec2(gl_GlobalInvocationID.xy), color);
}