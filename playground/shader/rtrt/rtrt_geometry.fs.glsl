#include "renderer/shader/version.glsl"

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec4 out_position_ws;
layout (location = 2) out vec4 out_surface_normal;

uniform vec4 color;
in vec3 gf_surface_normal_ws;
in vec3 gf_position_ws;

void main()
{
  out_color = color;
  out_surface_normal = vec4(gf_surface_normal_ws, 0.0);
  out_position_ws = vec4(gf_position_ws, 0.0);
}