#version 330 core

layout (triangles) in;
layout (line_strip, max_vertices = 18) out;

uniform float length;
uniform bool show_normal;
uniform bool show_TBN;
uniform bool show_triangle;

out vec4 color_;

in mat4 mvp_[];
in vec3 normal_[];
in vec4 pos_[];
in vec3 pos_bug_[];
in mat3 TBN_[];

void GenNormal(int index) {
  color_ = vec4(1, 1, 0, 1);
  gl_Position = mvp_[0] * pos_[index];
  EmitVertex();
  gl_Position = mvp_[0] * (pos_[index] + vec4(normal_[index], 0.0) * length);
  EmitVertex();
  EndPrimitive();
}

void GenTriangle(int index0, int index1) {
  color_ = vec4(1, 1, 1, 1);
  gl_Position = mvp_[0] * pos_[index0];                   // Right
  // gl_Position = mvp_[0] * vec4(pos_bug_[index0], 1.0); // Wrong
  EmitVertex();
  gl_Position = mvp_[0] * pos_[index1];                   // Right
  // gl_Position = mvp_[0] * vec4(pos_bug_[index1], 1.0); // Wrong
  EmitVertex();
  EndPrimitive();
}

void GenTBN(vec4 TBN_coord_pos, vec3 TBN_coord_axis, vec4 color) {
  color_ = color;
  gl_Position = mvp_[0] * TBN_coord_pos;
  EmitVertex();
  gl_Position = mvp_[0] * (TBN_coord_pos + vec4(TBN_coord_axis, 0.0) * length);
  EmitVertex();
  EndPrimitive();
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
}