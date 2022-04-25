#version 410 core

uniform samplerCube texture0;

in vec3 texcoord_;
out vec4 FragColor;

void main()
{
  FragColor = texture(texture0, texcoord_);
}