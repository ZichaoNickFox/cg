#include "renderer/shader/version.glsl"

#include "renderer/shader/camera.glsl"
#include "renderer/shader/transform.glsl"

layout (location = 0) in vec3 position_ls;
layout (location = 1) in vec3 normal_ls;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in vec3 tangent_ls;
layout (location = 4) in vec3 bitangent_ls;

uniform mat4 model;
uniform Camera camera;

out vec3 vf_position_vs;
out vec3 vf_normal_vs;

void main()
{
  vf_position_vs = PositionLS2VS(model, camera.view, position_ls);
  vf_normal_vs = NormalLS2VS(model, camera.view, normal_ls);
  gl_Position = PositionVS2CS(camera.project, vf_position_vs);
}