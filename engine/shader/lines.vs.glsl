#version 430 core

layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;

out vec3 color_;
out vec3 pos_ws_;

void main()
{
  color_ = color.xyz;
  gl_Position = project * view * model * pos;
  pos_ws_ = vec3(model * pos);
}