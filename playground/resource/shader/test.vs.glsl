#version 330 core

layout (location = 0) in vec3 pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;

out vec3 color_;

void main()
{
  color_ = pos;
  gl_Position = project * view * model * vec4(pos, 1.0f);
}