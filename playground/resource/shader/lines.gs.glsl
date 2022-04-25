#version 410 core

layout (lines) in;
layout (triangle_strip, max_vertices = 6) out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;
uniform vec3 view_pos;
uniform float line_width;

in vec3 color_[];
in vec3 pos_ws_[];
out vec3 color_fs_;

void LineWidth(vec3 pos0_ws, vec3 pos1_ws, vec3 color0, vec3 color1, vec3 view_pos, float width) {
  vec3 extension_dir = normalize(cross(pos1_ws - pos0_ws, view_pos - pos0_ws));
  mat4 vp = project * view;
  float scaled_width = width / 500.0;
  color_fs_ = color0;
  gl_Position = vp * vec4(pos0_ws + extension_dir * scaled_width, 1.0);
  EmitVertex();
  color_fs_ = color0;
  gl_Position = vp * vec4(pos0_ws - extension_dir * scaled_width, 1.0);
  EmitVertex();
  color_fs_ = color1;
  gl_Position = vp * vec4(pos1_ws + extension_dir * scaled_width, 1.0);
  EmitVertex();
  color_fs_ = color0;
  gl_Position = vp * vec4(pos0_ws - extension_dir * scaled_width, 1.0);
  EmitVertex();
  color_fs_ = color1;
  gl_Position = vp * vec4(pos1_ws - extension_dir * scaled_width, 1.0);
  EmitVertex();
  color_fs_ = color1;
  gl_Position = vp * vec4(pos1_ws + extension_dir * scaled_width, 1.0);
  EmitVertex();
  EndPrimitive();
}

void main()
{
  LineWidth(pos_ws_[0], pos_ws_[1], color_[0], color_[1], view_pos, line_width);
}