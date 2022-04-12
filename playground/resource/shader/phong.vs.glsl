#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;

out vec2 texcoord_;
out vec3 frag_world_pos_;
out vec3 normal_;
out mat3 TBN_;

void main()
{
  texcoord_ = texcoord;
  frag_world_pos_ = mat3(model) * pos;
  normal_ = mat3(model) * normal;

  TBN_[0] = tangent;
  TBN_[1] = bitangent;
  TBN_[2] = normal_;

  gl_Position = project * view * model * vec4(pos, 1.0);
}