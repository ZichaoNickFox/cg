#version 410 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texcoord;
layout (location = 2) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;

out vec2 texcoord_;
out vec3 frag_world_pos_;
out vec3 normal_;

// shadow map
uniform mat4 shadow_map_vp;
out vec4 shadow_map_clip_pos_;

void main()
{
  texcoord_ = texcoord;
  frag_world_pos_ = (model * vec4(pos, 1.0)).xyz;
  normal_ = normal;
  gl_Position = project * view * model * vec4(pos, 1.0);

  // shadow map
  shadow_map_clip_pos_ = (shadow_map_vp * model * vec4(pos, 1.0));
}