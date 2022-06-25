#include "renderer/shader/version.glsl"

#include "renderer/shader/camera.glsl"
#include "renderer/shader/convert.glsl"

layout (lines) in;
layout (triangle_strip, max_vertices = 6) out;

uniform Camera camera;
uniform float line_width;

in vec4 color_[];
in vec3 pos_ws_[];
out vec4 color_fs_;

void LineWidth(vec3 pos0_ws, vec3 pos1_ws, vec4 color0, vec4 color1, float width) {
  vec3 extension_dir = normalize(cross(pos1_ws - pos0_ws, camera.pos_ws - pos0_ws));
  mat4 view_project = camera.project * camera.view;
  float scaled_width = width / 500.0;
  color_fs_ = color0;
  gl_Position = PositionWS2CS(view_project, pos0_ws + extension_dir * scaled_width);
  EmitVertex();
  color_fs_ = color0;
  gl_Position = PositionWS2CS(view_project, pos0_ws - extension_dir * scaled_width);
  EmitVertex();
  color_fs_ = color0;
  gl_Position = PositionWS2CS(view_project, pos1_ws + extension_dir * scaled_width);
  EmitVertex();
  color_fs_ = color0;
  gl_Position = PositionWS2CS(view_project, pos0_ws - extension_dir * scaled_width);
  EmitVertex();
  color_fs_ = color0;
  gl_Position = PositionWS2CS(view_project, pos1_ws - extension_dir * scaled_width);
  EmitVertex();
  color_fs_ = color0;
  gl_Position = PositionWS2CS(view_project, pos1_ws + extension_dir * scaled_width);
  EmitVertex();
  EndPrimitive();
}

void main()
{
  LineWidth(pos_ws_[0], pos_ws_[1], color_[0], color_[1], line_width);
}