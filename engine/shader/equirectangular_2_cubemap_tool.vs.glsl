#version 430 core

layout (location = 0) in vec3 pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;

out vec3 local_pos_;

void main()
{
  local_pos_ = pos;
  gl_Position = project * view * model * vec4(local_pos_, 1.0);
}