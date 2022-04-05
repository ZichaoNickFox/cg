#version 330 core

uniform sampler2D texture0;

in vec2 texcoord_;
in vec4 position_;

out vec4 FragColor;

void main()
{
  float depth = texture(texture0, texcoord_).r;
  FragColor = vec4(depth, depth, depth, 1.0);
  FragColor = vec4(1,0,0,1);
}