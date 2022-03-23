#version 330 core

uniform sampler2D screen_texture;

in vec2 texcoord_;
out vec4 color;

void main() {
  float depth = texture(screen_texture, texcoord_).r;
  color = vec4(depth, depth, depth, 1.0);
}