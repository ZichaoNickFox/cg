#version 410 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;

out vec3 texcoord_;

void main()
{
  if (normal.x != 0) {
    texcoord_ = vec3(normal.x, (texcoord - 0.5) * 2);
  } else if (normal.y != 0) {
    texcoord_ = vec3((texcoord.x - 0.5) * 2, normal.y, (texcoord.y - 0.5) * 2);
  } else if (normal.z != 0) {
    texcoord_ = vec3((texcoord - 0.5) * 2, normal.z);
  } else {
    1 / 0;
  }
  gl_Position = project * view * model * vec4(pos, 1.0);
}