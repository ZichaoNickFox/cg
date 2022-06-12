#include "engine/shader/version.glsl"

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;

out vec3 frag_world_pos_;
out vec3 normal_ws_;
out vec2 texcoord_;
out mat3 world_TBN_;

out mat4 model_;

void main()
{
  frag_world_pos_ = (model * vec4(pos, 1.0)).xyz;
  normal_ws_ = (model * vec4(normal, 1.0)).xyz;

  world_TBN_[0] = normalize(vec3(model * vec4(tangent, 0.0)));
  world_TBN_[1] = normalize(vec3(model * vec4(bitangent, 0.0)));
  world_TBN_[2] = normalize(normal_ws_);

  texcoord_ = texcoord;

  gl_Position = project * view * model * vec4(pos, 1.0);
}