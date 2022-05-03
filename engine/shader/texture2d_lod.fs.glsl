#version 410 core

uniform sampler2D texture2D0;
uniform vec3 view_pos_ws;

in vec2 texcoord_;
in vec3 frag_pos_ws_;
out vec4 FragColor;

void main()
{
  float dist = distance(view_pos_ws, frag_pos_ws_);
  FragColor = textureLod(texture2D0, texcoord_, dist / 2);
}