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

out vec3 TBN_tangent_dir_;
out vec3 TBN_bitangent_dir_;
out vec3 TBN_normal_dir_;

void main()
{
  mvp_ = project * view * model;
  normal_ = normal;
  pos_ = vec4(pos, 1.0);
  pos_bug_ = pos;

  TBN_tangent_dir_ = tangent;
  TBN_bitangent_dir_ = bitangent;
  TBN_normal_dir_ = normal_;
}
