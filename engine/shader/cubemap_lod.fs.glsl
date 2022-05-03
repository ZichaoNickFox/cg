#version 410 core

uniform samplerCube texture_cubemap;
uniform vec3 view_pos_ws;

in vec3 frag_pos_ws_;
in vec3 frag_pos_ls_;
out vec4 FragColor;

void main()
{
  float dist = distance(view_pos_ws, frag_pos_ws_);
  FragColor = textureLod(texture_cubemap, frag_pos_ls_, dist / 2);
}