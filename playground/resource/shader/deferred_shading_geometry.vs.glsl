#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;

out vec2 texcoord_;
out vec3 normal_;
out vec4 position_;
out vec4 frag_world_pos_;

void main()
{
  texcoord_ = texcoord;
  normal_ = normal;
  position_ = project * view * model * vec4(position, 1.0);
  frag_world_pos_ = model * vec4(position, 1.0);
  gl_Position = position_;
}