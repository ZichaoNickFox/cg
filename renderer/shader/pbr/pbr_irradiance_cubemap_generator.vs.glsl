#version 430 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;

out vec3 local_pos_;
out vec3 normal_;

void main()
{
  local_pos_ = pos;
  // Use position as normal
  normal_ = local_pos_;
  gl_Position = project * view * model * vec4(local_pos_, 1.0);
}