#include "renderer/shader/version.glsl"

#include "renderer/shader/camera.glsl"
#include "renderer/shader/transform.glsl"

layout (location = 0) in vec3 position_ls;
layout (location = 1) in vec3 normal_ls;

uniform mat4 model;
uniform Camera camera;
uniform int primitive_start_index;

out vec3 vg_position_ws;
out vec4 vg_position_cs;
out flat int vg_primitive_index;

void main()
{
  vg_position_ws = PositionLS2WS(model, position_ls);
  vg_position_cs = PositionLS2CS(model, camera.view, camera.project, position_ls);
  vg_primitive_index = primitive_start_index + gl_VertexID / 3;
}