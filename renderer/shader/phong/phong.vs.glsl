#include "renderer/shader/version.glsl"

#include "renderer/shader/camera.glsl"
#include "renderer/shader/transform.glsl"

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;
layout (location = 5) in vec4 color;

uniform Camera camera;
uniform mat4 model;

out vec2 texcoord_;
out vec3 frag_world_pos_;
out vec3 frag_world_normal_;
out mat3 world_TBN_;

void main()
{
  texcoord_ = texcoord;
  frag_world_pos_ = PositionLS2WS(model, pos);
  frag_world_normal_ = NormalLS2WS(model, normal);

  world_TBN_[0] = normalize(vec3(model * vec4(tangent, 0.0)));
  world_TBN_[1] = normalize(vec3(model * vec4(bitangent, 0.0)));
  world_TBN_[2] = normalize(vec3(model * vec4(normal, 0.0)));

  gl_Position = camera.project * camera.view * model * vec4(pos, 1.0);
}