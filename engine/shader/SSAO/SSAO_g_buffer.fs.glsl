#version 410 core

layout (location = 0) out vec4 position_vs;
layout (location = 1) out vec3 normal_vs;
layout (location = 2) out vec4 albedo;

in vec2 texcoord_;
in vec4 position_vs_;
in vec4 normal_vs_;

void main() {
  position_vs = vec4(position_vs_.z,position_vs_.z, position_vs_.z,1.0);
  normal_vs = normalize(normal_vs_.xyz);
  albedo = vec4(0.95, 0.95, 0.95, 1.0);
}
