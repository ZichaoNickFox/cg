#include "renderer/shader/version.glsl"

#include "renderer/shader/camera.glsl"
#include "renderer/shader/depth.glsl"
#include "renderer/shader/transform.glsl"

layout (location = 0) out vec4 out_position_vs;
layout (location = 1) out vec4 out_normal_vs;

in vec3 vf_position_vs;
in vec3 vf_normal_vs;

void main() {
  out_position_vs = vec4(vf_position_vs, 1.0);
  out_normal_vs = vec4(vf_normal_vs, 1.0);
}
