#version 410 core

layout (location = 0) out vec4 position_ws;
layout (location = 1) out vec3 normal_vs;
layout (location = 2) out vec4 albedo;

in vec2 texcoord_;
in vec4 position_ws_;
in vec4 normal_vs_;

void main() {
  position_ws = position_ws_;
  normal_vs = normalize(normal_vs_.xyz);
  albedo = vec4(0.95, 0.95, 0.95, 1.0);
}
