#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;

out vec3 color_;
out vec3 pos_ws_;

void main()
{
  color_ = color;
  gl_Position = project * view * model * vec4(pos, 1.0);
  pos_ws_ = vec3(model * vec4(pos, 1.0));
}