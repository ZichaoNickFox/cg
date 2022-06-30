#include "renderer/shader/version.glsl"

#include "renderer/shader/camera.glsl"

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

uniform mat4 model;
uniform Camera camera;

out vec3 normal_;
out vec3 world_pos_;
out vec3 world_normal_;
out vec2 texcoord_;

out mat3 world_TBN_;

void main()
{
  texcoord_ = texcoord;
  normal_ = normal;
  world_pos_ = (model * vec4(pos, 1.0)).xyz;
  world_normal_ = (model * vec4(normal, 1.0)).xyz;

  world_TBN_[0] = normalize(vec3(model * vec4(tangent, 0.0)));
  world_TBN_[1] = normalize(vec3(model * vec4(bitangent, 0.0)));
  world_TBN_[2] = world_normal_;
}