#version 330 core

out vec4 FragColor;

in vec2 texcoord_;

void main()
{
  FragColor = vec4(texcoord_, 0, 1);
}