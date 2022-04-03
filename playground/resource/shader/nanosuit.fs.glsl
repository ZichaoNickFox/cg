#version 330 core

uniform sampler2D diffuse_texture0;
uniform sampler2D specular_texture0;

in vec2 texcoord_;
out vec4 FragColor;

void main()
{
  FragColor = texture(specular_texture0, texcoord_);
  FragColor = texture(diffuse_texture0, texcoord_);
}