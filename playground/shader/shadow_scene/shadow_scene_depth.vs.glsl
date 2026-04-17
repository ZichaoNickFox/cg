#include "renderer/shader/version.glsl"

#include "renderer/shader/transform.glsl"

layout (location = 0) in vec3 pos;

uniform mat4 model;
uniform mat4 light_view_project;

void main()
{
  vec3 position_ws = PositionLS2WS(model, pos);
  gl_Position = PositionWS2CS(light_view_project, position_ws);
}
