#include "engine/shader/version.glsl"
#include "engine/shader/convert.glsl"

layout (location = 0) in vec3 position_ls;
layout (location = 1) in vec3 normal_ls;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in vec3 tangent_ls;
layout (location = 4) in vec3 bitangent_ls;

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;

out vec2 texcoord_;
out vec4 position_vs_;
out vec3 normal_vs_;

void main()
{
  texcoord_ = texcoord;

  position_vs_ = view * model * vec4(position_ls, 1.0);
  normal_vs_ = NormalLS2VS(model, view, normal_ls);
  gl_Position = project * view * model * vec4(position_ls, 1.0);
}