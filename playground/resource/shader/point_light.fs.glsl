#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

uniform vec3 light_color;

void main()
{
  FragColor = vec4(light_color, 1.0f);
  BrightColor = vec4(light_color, 1.0f);
}