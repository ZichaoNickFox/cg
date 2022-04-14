#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;

out vec3 normal_;
out vec4 pos_;
out vec3 pos_bug_;
out mat4 mvp_;

out mat3 TBN_;

void main()
{
  mvp_ = project * view * model;
  normal_ = normal;
  pos_ = vec4(pos, 1.0);
  pos_bug_ = pos;

  TBN_ = mat3(tangent, bitangent, normal_);
}
