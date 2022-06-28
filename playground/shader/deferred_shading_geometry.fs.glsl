#version 430 core

layout (location = 0) out vec4 position;
layout (location = 1) out vec4 normal;
layout (location = 2) out vec4 texcoord;
layout (location = 3) out vec4 frag_world_pos;

in vec4 position_;
in vec3 normal_;
in vec2 texcoord_;
in vec4 frag_world_pos_;

void main()
{
  position = position_;
  normal = vec4(normal_, 1.0);
  texcoord = vec4(texcoord_, 0.0, 1.0);
  frag_world_pos = frag_world_pos_;
}