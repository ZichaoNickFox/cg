#version 430 core

out vec4 FragColor;

in vec4 color_;

void main()
{
  FragColor = color_;
}