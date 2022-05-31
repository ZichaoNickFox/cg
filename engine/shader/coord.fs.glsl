#version 430 core

in vec3 color_;

out vec4 FragColor;

void main()
{
  FragColor = vec4(color_, 1);
}