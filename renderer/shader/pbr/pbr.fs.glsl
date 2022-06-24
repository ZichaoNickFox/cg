#include "renderer/shader/version.glsl"

#include "renderer/shader/light_info.glsl"
#include "renderer/shader/pbr/pbr_fresnel.glsl"
#include "renderer/shader/pbr/pbr_geometry.glsl"
#include "renderer/shader/pbr/pbr_material.glsl"
#include "renderer/shader/pbr/pbr_model.glsl"
#include "renderer/shader/pbr/pbr_NDF.glsl"

out vec4 FragColor;

in vec3 frag_world_pos_;
in vec3 normal_ws_;

uniform vec3 view_pos_ws;
uniform PbrMaterial pbr_material;

in mat3 world_TBN_;

in vec2 texcoord_;

in mat4 model_;

void main()
{
  vec3 N = vec3(0, 0, 0);
  if (pbr_material.use_texture_normal) {
    vec3 normal_from_texture = texture(pbr_material.texture_normal0, texcoord_).xyz;
    normal_from_texture = normal_from_texture * 2.0 - 1.0;
    N = world_TBN_ * normal_from_texture;
  } else {
    N = normal_ws_;
  }

  vec3 albedo = vec3(0, 0, 0);
  if (pbr_material.use_texture_albedo) {
    albedo = texture(pbr_material.texture_albedo0, texcoord_).xyz;
  } else {
    albedo = pbr_material.albedo;
  }

  float roughness = 0.0;
  if (pbr_material.use_texture_roughness) {
    roughness = texture(pbr_material.texture_roughness0, texcoord_).x;
  } else {
    roughness = pbr_material.roughness;
  }

  float metallic = 0.0;
  if (pbr_material.use_texture_metallic) {
    metallic = texture(pbr_material.texture_metallic0, texcoord_).x;
  } else {
    metallic = pbr_material.metallic;
  }

  vec3 ao = vec3(0.0, 0.0, 0.0);
  if (pbr_material.use_texture_ao) {
    ao = texture(pbr_material.texture_ao0, texcoord_).xyz;
  } else {
    ao = pbr_material.ao;
  }

  PbrModelInput pbr_model_input;
  pbr_model_input.view_pos_ws = view_pos_ws;
  pbr_model_input.frag_world_pos = frag_world_pos_;
  pbr_model_input.albedo = albedo;
  pbr_model_input.roughness = roughness;
  pbr_model_input.metallic = metallic;
  pbr_model_input.ao = ao;
  pbr_model_input.normal = N;

  vec3 color = PbrModel(pbr_model_input);
  // color = color / (color + vec3(1.0));
  // color = pow(color, vec3(1.0 / 2.2));

  FragColor = vec4(color, 1.0);
}