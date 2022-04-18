#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 108) out;

uniform mat4 project;
uniform mat4 view;
uniform mat4 model;
uniform float line_length;
uniform float line_width;
uniform bool show_vertex_normal;
uniform bool show_TBN;
uniform bool show_texture_normal;
uniform bool show_triangle;
uniform vec3 view_pos;
uniform sampler2D texture_normal;

out vec4 color_;

in mat4 vp_[];
in vec3 pos_bug_[];
in mat3 world_TBN_[];
in vec3 world_normal_[];
in vec3 world_pos_[];
in vec2 texcoord_[];

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

void GenVertexNormal(int index) {
  vec3 color = vec3(1, 1, 0);
  vec3 pos0_ws = world_pos_[index];
  vec3 pos1_ws = world_pos_[index] + world_normal_[index] * line_length;
  LineWidth(pos0_ws, pos1_ws, color, color, view_pos, line_width);
}

void GenTextureNormal(int index) {
  vec3 color = vec3(0, 1, 1);
  vec3 pos0_ws = world_pos_[index];
  vec3 normal_from_texture = texture(texture_normal, texcoord_[index]).xyz;
  normal_from_texture = normalize(normal_from_texture * 2.0 - 1.0);
  vec3 pos1_ws = world_pos_[index] + normalize(world_TBN_[index] * normal_from_texture) * line_length;
  LineWidth(pos0_ws, pos1_ws, color, color, view_pos, line_width);
}

void GenTriangle(int index0, int index1) {
  vec3 color = vec3(1, 1, 1);
  vec3 pos0_ws = world_pos_[index0];
  vec3 pos1_ws = world_pos_[index1];
  LineWidth(pos0_ws, pos1_ws, color, color, view_pos, line_width);
}

void GenTBN(vec3 TBN_coord_world_pos, vec3 TBN_coord_world_axis, vec4 color) {
  vec3 pos0_ws = TBN_coord_world_pos;
  vec3 pos1_ws = TBN_coord_world_pos + TBN_coord_world_axis * line_length;
  LineWidth(pos0_ws, pos1_ws, vec3(color), vec3(color), view_pos, line_width);
}

void main()
{
  if (show_vertex_normal) {
    GenVertexNormal(0);
    GenVertexNormal(1);
    GenVertexNormal(2);
  }
  if (show_texture_normal) {
    GenTextureNormal(0);
    GenTextureNormal(1);
    GenTextureNormal(2);
  }
  if (show_triangle) {
    GenTriangle(0, 1);
    GenTriangle(1, 2);
    GenTriangle(0, 2);
  }
  if (show_TBN) {
    vec3 TBN_coord_world_pos = (world_pos_[0] + world_pos_[1] + world_pos_[2]) / 3;
    GenTBN(TBN_coord_world_pos, world_TBN_[0][0], vec4(1, 0, 0, 1));
    GenTBN(TBN_coord_world_pos, world_TBN_[0][1], vec4(0, 1, 0, 1));
    GenTBN(TBN_coord_world_pos, world_TBN_[0][2], vec4(0, 0, 1, 1));
  }
}