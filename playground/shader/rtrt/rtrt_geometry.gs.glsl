#include "renderer/shader/version.glsl"

#include "renderer/shader/camera.glsl"
#include "renderer/shader/color.glsl"
#include "renderer/shader/line.glsl"

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 model;
uniform Camera camera;

in vec3 vg_position_ws[];
in vec4 vg_position_cs[];
in flat int vg_primitive_index[];

out vec3 gf_surface_normal_ws;
out vec3 gf_position_ws;
out flat int gf_primitive_index;

void main()
{
  gf_surface_normal_ws = SurfaceNormal(vg_position_ws[0], vg_position_ws[1], vg_position_ws[2]);
  gf_primitive_index = vg_primitive_index[0];

  gf_position_ws = vg_position_ws[0];
  gl_Position = vg_position_cs[0];
  EmitVertex();

  gf_position_ws = vg_position_ws[1];
  gl_Position = vg_position_cs[1];
  EmitVertex();

  gf_position_ws = vg_position_ws[2];
  gl_Position = vg_position_cs[2];
  EmitVertex();
}