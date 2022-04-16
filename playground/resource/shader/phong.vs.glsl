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
out vec3 frag_world_normal_;
out mat3 world_TBN_;
out mat4 model_;

void main()
{
  texcoord_ = texcoord;
  frag_world_pos_ = mat3(model) * pos;
  frag_world_normal_ = mat3(model) * normal;

  world_TBN_[0] = normalize(vec3(model * vec4(tangent, 0.0)));
  world_TBN_[1] = normalize(vec3(model * vec4(bitangent, 0.0)));
  world_TBN_[2] = normalize(vec3(model * vec4(normal, 0.0)));

  model_ = model;

  gl_Position = project * view * model * vec4(pos, 1.0);
}