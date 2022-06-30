#include "renderer/shader/version.glsl"

#include "renderer/shader/camera.glsl"
#include "renderer/shader/convert.glsl"
#include "renderer/shader/line.glsl"

layout (lines) in;
layout (triangle_strip, max_vertices = 6) out;

uniform Camera camera;
uniform float line_width;

in vec4 color_[];
in vec3 pos_ws_[];

void main()
{
  GenLineGS(camera, pos_ws_[0], pos_ws_[1], color_[0], color_[1], line_width);
}