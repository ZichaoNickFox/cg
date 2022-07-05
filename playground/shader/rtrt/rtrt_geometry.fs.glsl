#include "renderer/shader/version.glsl"

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec4 out_emission;
layout (location = 2) out vec4 out_position_ws;
layout (location = 3) out vec4 out_surface_normal;

uniform vec4 color;
uniform vec4 emission;
in vec3 gf_surface_normal_ws;
in vec3 gf_position_ws;

void main()
{
  out_color = color;
  out_emission = emission;
  out_surface_normal = vec4(gf_surface_normal_ws, 1.0);
  out_position_ws = vec4(gf_position_ws, 1.0);
}