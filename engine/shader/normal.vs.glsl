#version 410 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;

out vec3 normal_;
out vec3 world_pos_;
out vec3 world_normal_;
out vec2 texcoord_;

out mat3 world_TBN_;

void main()
{
  texcoord_ = texcoord;
  normal_ = normal;
  world_pos_ = (model * vec4(pos, 1.0)).xyz;
  world_normal_ = (model * vec4(normal, 0.0)).xyz;

  vec3 world_tangent = mat3(model) * tangent;
  vec3 world_bitangent = mat3(model) * bitangent;
  world_TBN_ = mat3(world_tangent, world_bitangent, world_normal_);
}