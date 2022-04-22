layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;

out vec3 frag_world_pos_;
out vec3 normal_;
out vec2 texcoord_;
out mat3 world_TBN_;

uniform ShadowInfo shadow_info;
uniform bool use_shadowing = false;
out vec4 frag_shadow_pos_;

void main()
{
  frag_world_pos_ = (model * vec4(pos, 1.0)).xyz;
  normal_ = normal;
  gl_Position = project * view * model * vec4(pos, 1.0);

  world_TBN_[0] = normalize(vec3(model * vec4(tangent, 0.0)));
  world_TBN_[1] = normalize(vec3(model * vec4(bitangent, 0.0)));
  world_TBN_[2] = normalize(vec3(model * vec4(normal, 0.0)));

  texcoord_ = texcoord;
  
  if (use_shadowing) {
    frag_shadow_pos_ = shadow_info.light_space_vp * model * vec4(pos, 1.0);
  }
}