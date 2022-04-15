#version 330 core

in vec3 color_fs_;

out vec4 FragColor;

void main()
{
  FragColor = vec4(color_fs_, 1);
}