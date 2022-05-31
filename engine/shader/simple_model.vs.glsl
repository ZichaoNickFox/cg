#version 430 core

layout (location = 0) in vec3 position_ls;
layout (location = 1) in vec3 normal_ls;
layout (location = 2) in vec2 texcoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;

out vec2 texcoord_;

void main() {
  texcoord_ = texcoord;
  gl_Position = project * view * model * vec4(position_ls, 1.0);
}
