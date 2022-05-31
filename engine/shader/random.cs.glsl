#version 430 core

out vec4 color;
in vec2 texcoord_;

float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233)))* 43758.5453123);
}

void main() {
  float r = random(texcoord_);
  color = vec4(r, r, r, 1);
}