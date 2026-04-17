#include "renderer/shader/version.glsl"

#include "renderer/shader/camera.glsl"
#include "renderer/shader/transform.glsl"

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

uniform Camera camera;
uniform mat4 model;
uniform mat4 light_view_project;

out vec2 texcoord_;
out vec3 position_ws_;
out vec3 normal_ws_;
out vec4 position_light_cs_;
out mat3 TBN_ws_;

void main()
{
  texcoord_ = texcoord;
  position_ws_ = PositionLS2WS(model, pos);
  normal_ws_ = NormalLS2WS(model, normal);
  position_light_cs_ = PositionWS2CS(light_view_project, position_ws_);

  TBN_ws_[0] = normalize(vec3(model * vec4(tangent, 0.0)));
  TBN_ws_[1] = normalize(vec3(model * vec4(bitangent, 0.0)));
  TBN_ws_[2] = normalize(vec3(model * vec4(normal, 0.0)));

  gl_Position = PositionWS2CS(camera.view, camera.project, position_ws_);
}
