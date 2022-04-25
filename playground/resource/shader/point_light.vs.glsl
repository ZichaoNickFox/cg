#version 410 core

layout (location = 0) in vec3 pos;
layout (location = 2) in vec2 texcoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;

void main()
{
  gl_Position = project * view * model * vec4(pos, 1.0);
}