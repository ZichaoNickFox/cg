#version 330 core

in vec2 texcoord;
out vec4 color;

uniform sampler2D screenTexture;

void main(){
  color = texture2D(screenTexture, texcoord);
}
