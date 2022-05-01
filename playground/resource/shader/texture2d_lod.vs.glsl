#version 410 core

layout (location = 0) in vec3 pos;
layout (location = 2) in vec2 texcoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;

out vec2 texcoord_;
out vec3 frag_pos_ws_;
out vec3 frag_pos_ls_;

void main()
{
  frag_pos_ws_ = vec3(model * vec4(pos, 1.0));
  texcoord_ = texcoord;
  gl_Position = project * view * model * vec4(pos, 1.0);
}