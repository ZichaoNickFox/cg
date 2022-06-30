#include "renderer/shader/version.glsl"

#include "renderer/shader/camera.glsl"
#include "renderer/shader/color.glsl"
#include "renderer/shader/line.glsl"

layout (triangles) in;
layout (triangle_strip, max_vertices = 108) out;

uniform mat4 model;
uniform Camera camera;

uniform float line_length;
uniform float line_width;
uniform bool show_vertex_normal;
uniform bool show_TBN;
uniform bool show_texture_normal;
uniform bool show_triangle;
uniform bool show_face_normal;
uniform vec3 view_pos_ws;
uniform sampler2D texture_normal;

in mat3 world_TBN_[];
in vec3 world_normal_[];
in vec3 world_pos_[];
in vec2 texcoord_[];

void GenVertexNormal(int index) {
  vec4 color = kPurple;
  vec3 pos0_ws = world_pos_[index];
  vec3 pos1_ws = world_pos_[index] + world_normal_[index] * line_length;
  GenLineGS(camera, pos0_ws, pos1_ws, color, color, line_width);
}

void GenVertexTextureNormal(int index) {
  vec4 color = kYellow;
  vec3 pos0_ws = world_pos_[index];
  vec3 normal_from_texture = texture(texture_normal, texcoord_[index]).xyz;
  normal_from_texture = normalize(normal_from_texture * 2.0 - 1.0);
  vec3 pos1_ws = world_pos_[index] + normalize(world_TBN_[index] * normal_from_texture) * line_length;
  GenLineGS(camera, pos0_ws, pos1_ws, color, color, line_width);
}

void GenTriangle(int index0, int index1) {
  vec4 color = kWhite;
  vec3 pos0_ws = world_pos_[index0];
  vec3 pos1_ws = world_pos_[index1];
  GenLineGS(camera, pos0_ws, pos1_ws, color, color, line_width);
}

void GenTBN(vec3 TBN_coord_world_pos, vec3 TBN_coord_world_axis, vec4 color) {
  vec3 pos0_ws = TBN_coord_world_pos;
  vec3 pos1_ws = TBN_coord_world_pos + TBN_coord_world_axis * line_length;
  GenLineGS(camera, pos0_ws, pos1_ws, color, color, line_width);
}

void GenFaceNormal() {
  vec4 color = kCyan;
  vec3 normal = cross(world_pos_[1] - world_pos_[0], world_pos_[2] - world_pos_[0]);
  vec3 center = (world_pos_[0] + world_pos_[1] + world_pos_[2]) / 3;
  GenLineGS(camera, center, center + normalize(normal) * line_length, color, color, line_width);
}

void main()
{
  if (show_vertex_normal) {
    GenVertexNormal(0);
    GenVertexNormal(1);
    GenVertexNormal(2);
  }
  if (show_face_normal) {
    GenFaceNormal();
  }
  if (show_texture_normal) {
    GenVertexTextureNormal(0);
    GenVertexTextureNormal(1);
    GenVertexTextureNormal(2);
  }
  if (show_TBN) {
    vec3 TBN_coord_world_pos = (world_pos_[0] + world_pos_[1] + world_pos_[2]) / 3;
    GenTBN(TBN_coord_world_pos, world_TBN_[0][0], kRed);
    GenTBN(TBN_coord_world_pos, world_TBN_[0][1], kGreen);
    GenTBN(TBN_coord_world_pos, world_TBN_[0][2], kBlue);
  }
  if (show_triangle) {
    GenTriangle(0, 1);
    GenTriangle(1, 2);
    GenTriangle(0, 2);
  }
}