#version 410 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texcoord;
layout (location = 2) in vec3 color;

out vec3 pos_;
out vec2 texcoord_;
out vec3 color_;

void main()
{
  texcoord_ = texcoord;
  color_ = color;
  gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);
}