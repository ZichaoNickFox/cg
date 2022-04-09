#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;

out vec2 texcoord_;
out vec3 frag_world_pos_;
out vec3 normal_;

void main()
{
  texcoord_ = texcoord;
  frag_world_pos_ = (model * vec4(pos, 1.0)).xyz;
  normal_ = normal;
  gl_Position = project * view * model * vec4(pos, 1.0);
}