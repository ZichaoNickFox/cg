#version 430 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;

out vec3 texcoord_;

void main()
{
  texcoord_ = pos;
  gl_Position = project * view * model * vec4(pos, 1.0);
}