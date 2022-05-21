#version 410 core

layout (location = 0) in vec3 position_ls;
layout (location = 1) in vec2 texcoord;
layout (location = 2) in vec3 normal_ls;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;

out vec2 texcoord_;
out vec4 position_ws_;
out vec4 normal_vs_;

void main()
{
  texcoord_ = texcoord;
  position_ws_ = model * vec4(position_ls, 1.0);
  normal_vs_ = view * model * vec4(normal_ls, 1.0);
  gl_Position = project * view * model * vec4(position_ls, 1.0);
}