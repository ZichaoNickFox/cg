#include "renderer/shader/version.glsl"

#include "renderer/shader/camera.glsl"
#include "renderer/shader/convert.glsl"

layout (location = 0) in vec3 pos;

uniform mat4 model;
uniform Camera camera;

void main()
{
  gl_Position = PositionLS2CS(model, camera.view, camera.project, pos);
}