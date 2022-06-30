#include "renderer/shader/camera.glsl"
#include "renderer/shader/convert.glsl"

out vec4 gs_output_color_;

void GenLineGS(Camera camera, vec3 pos0_ws, vec3 pos1_ws, vec4 color0, vec4 color1, float width) {
  vec3 extension_dir = normalize(cross(pos1_ws - pos0_ws, camera.pos_ws - pos0_ws));
  mat4 view_project = camera.project * camera.view;
  float scaled_width = width / 500.0;
  gs_output_color_ = color0;
  gl_Position = PositionWS2CS(view_project, pos0_ws + extension_dir * scaled_width);
  EmitVertex();
  gs_output_color_ = color0;
  gl_Position = PositionWS2CS(view_project, pos0_ws - extension_dir * scaled_width);
  EmitVertex();
  gs_output_color_ = color1;
  gl_Position = PositionWS2CS(view_project, pos1_ws + extension_dir * scaled_width);
  EmitVertex();
  gs_output_color_ = color0;
  gl_Position = PositionWS2CS(view_project, pos0_ws - extension_dir * scaled_width);
  EmitVertex();
  gs_output_color_ = color1;
  gl_Position = PositionWS2CS(view_project, pos1_ws - extension_dir * scaled_width);
  EmitVertex();
  gs_output_color_ = color1;
  gl_Position = PositionWS2CS(view_project, pos1_ws + extension_dir * scaled_width);
  EmitVertex();
  EndPrimitive();
}
