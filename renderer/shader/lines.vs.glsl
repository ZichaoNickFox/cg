#include "renderer/shader/version.glsl"

#include "renderer/shader/camera.glsl"
#include "renderer/shader/convert.glsl"

layout (location = 0) in vec3 pos;
layout (location = 5) in vec4 color;

uniform Camera camera;
uniform mat4 model;

out vec4 color_;
out vec3 pos_ws_;

void main()
{
  color_ = color;
  gl_Position = PositionLS2CS(model, camera.view, camera.project, pos);
  pos_ws_ = PositionLS2WS(model, pos);
}