#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;

out vec3 normal_;
out vec3 pos_;

void main()
{
  normal_ = normal;
  pos_ = pos;
}