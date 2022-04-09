#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;

void main()
{
  world_pos_ = model * vec4(pos, 1.0);
  normal_ = normal;
  gl_Position = project * view * model * vec4(pos, 1.0);
}