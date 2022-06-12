#include "engine/shader/version.glsl"

#include "engine/shader/convert.glsl"
#include "engine/shader/depth.glsl"

layout (location = 0) out vec3 position_vs;
layout (location = 1) out vec3 normal_vs;
layout (location = 2) out vec4 albedo;

in vec2 texcoord_;
in vec4 position_vs_;
in vec3 normal_vs_;

uniform float u_near;
uniform float u_far;

void main() {
  position_vs = vec3(position_vs_.x, position_vs_.y, position_vs_.z);
  normal_vs = normal_vs_;
  albedo = vec4(0.95, 0.95, 0.95, 1.0);
}
