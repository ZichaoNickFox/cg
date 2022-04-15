#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 108) out;

uniform mat4 project;
uniform mat4 view;
uniform mat4 model;
uniform float line_length;
uniform float line_width;
uniform bool show_normal;
uniform bool show_TBN;
uniform bool show_triangle;
uniform vec3 view_pos;

out vec4 color_;
out mat4 mvp_;

in mat4 vp_[];
in vec3 normal_[];
in vec4 pos_[];
in vec3 pos_bug_[];
in mat3 TBN_[];

void LineWidth(vec3 pos0_ws, vec3 pos1_ws, vec3 color0, vec3 color1, vec3 view_pos, float width) {
  vec3 extension_dir = normalize(cross(view_pos - pos0_ws, pos1_ws - pos0_ws));
  mat4 vp = project * view;
  float scaled_width = width / 500.0;
  color_ = vec4(color0, 1.0);
  gl_Position = vp * vec4(pos0_ws + extension_dir * scaled_width, 1.0);
  EmitVertex();
  color_ = vec4(color0, 1.0);
  gl_Position = vp * vec4(pos0_ws - extension_dir * scaled_width, 1.0);
  EmitVertex();
  color_ = vec4(color1, 1.0);
  gl_Position = vp * vec4(pos1_ws + extension_dir * scaled_width, 1.0);
  EmitVertex();
  color_ = vec4(color0, 1.0);
  gl_Position = vp * vec4(pos0_ws - extension_dir * scaled_width, 1.0);
  EmitVertex();
  color_ = vec4(color1, 1.0);
  gl_Position = vp * vec4(pos1_ws - extension_dir * scaled_width, 1.0);
  EmitVertex();
  color_ = vec4(color1, 1.0);
  gl_Position = vp * vec4(pos1_ws + extension_dir * scaled_width, 1.0);
  EmitVertex();
  EndPrimitive();
}

void GenNormal(int index) {
  vec3 color = vec3(1, 1, 0);
  vec3 pos0_ws = vec3(model * pos_[index]);
  vec3 pos1_ws = vec3(model * (pos_[index] + vec4(normal_[index], 0.0) * line_length));
  LineWidth(pos0_ws, pos1_ws, color, color, view_pos, line_width);
}

void GenTriangle(int index0, int index1) {
  vec3 color = vec3(1, 1, 1);
  vec3 pos0_ws = vec3(model * pos_[index0]);
  vec3 pos1_ws = vec3(model * pos_[index1]);
  LineWidth(pos0_ws, pos1_ws, color, color, view_pos, line_width);
}

void GenTBN(vec4 TBN_coord_pos, vec3 TBN_coord_axis, vec4 color) {
  vec3 pos0_ws = vec3(model * TBN_coord_pos);
  vec3 pos1_ws = vec3(model * (TBN_coord_pos + vec4(TBN_coord_axis, 0.0) * line_length));
  LineWidth(pos0_ws, pos1_ws, vec3(color), vec3(color), view_pos, line_width);
}

void main()
{
  if (show_normal) {
    GenNormal(0);
    GenNormal(1);
    GenNormal(2);
  }
  if (show_triangle) {
    GenTriangle(0, 1);
    GenTriangle(1, 2);
    GenTriangle(0, 2);
  }
  if (show_TBN) {
    vec4 TBN_coord_pos = (pos_[0] + pos_[1] + pos_[2]) / 3;
    GenTBN(TBN_coord_pos, TBN_[0][0], vec4(1, 0, 0, 1));
    GenTBN(TBN_coord_pos, TBN_[0][1], vec4(0, 1, 0, 1));
    GenTBN(TBN_coord_pos, TBN_[0][2], vec4(0, 0, 1, 1));
  }
  mvp_ = project * view * model;
}