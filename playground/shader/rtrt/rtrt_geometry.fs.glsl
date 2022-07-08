#include "renderer/shader/version.glsl"

#include "renderer/shader/color.glsl"

layout (location = 0) out vec4 out_position_ws;
layout (location = 1) out vec4 out_surface_normal;
layout (location = 2) out float out_primitive_index;

in vec3 gf_surface_normal_ws;
in vec3 gf_position_ws;
in flat int gf_primitive_index;

void main()
{
  out_surface_normal = vec4(gf_surface_normal_ws, 1.0);
  out_position_ws = vec4(gf_position_ws, 1.0);
  out_primitive_index = gf_primitive_index;
}