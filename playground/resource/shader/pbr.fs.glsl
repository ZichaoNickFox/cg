out vec4 FragColor;

in vec3 frag_world_pos_;
in vec3 normal_;

uniform vec3 view_pos;
uniform PbrMaterial pbr_material;

uniform float ao;

uniform sampler2D texture_albedo;

in mat3 world_TBN_;

in vec2 texcoord_;

uniform ShadowInfo shadow_info;
uniform bool use_shadowing = false;
in vec4 frag_shadow_pos_;

float CalcShadow() {
  if (use_shadowing) {
    ShadowModelInput shadow_model_input;
    shadow_model_input.frag_shadow_pos_light_space = frag_shadow_pos_;
    shadow_model_input.depth_in_texture = texture(shadow_info.texture_depth, GetShadowMapTexcoord(frag_shadow_pos_)).r;
    return ShadowModel(shadow_model_input);
  } else {
    return 1.0;
  }
}

void main()
{
  vec3 N = vec3(0, 0, 0);
  if (pbr_material.use_texture_normal) {
    vec3 normal_from_texture = texture(pbr_material.texture_normal0, texcoord_).xyz;
    normal_from_texture = normalize(normal_from_texture * 2.0 - 1.0);
    N = normalize(world_TBN_ * normal_from_texture);
  } else {
    N = normalize(normal_);
  }

  vec3 albedo = vec3(0, 0, 0);
  if (pbr_material.use_texture_albedo) {
    albedo = texture(pbr_material.texture_albedo0, texcoord_).xyz;
  } else {
    albedo = pbr_material.albedo;
  }

  PbrModelInput pbr_model_input;
  pbr_model_input.view_pos = view_pos;
  pbr_model_input.frag_world_pos = frag_world_pos_;
  pbr_model_input.albedo = albedo;
  pbr_model_input.roughness = pbr_material.roughness;
  pbr_model_input.metallic = pbr_material.metallic;
  pbr_model_input.ao = pbr_material.ao;
  pbr_model_input.normal = N;

  vec3 color = PbrModel(pbr_model_input);
  color = color / (color + vec3(1.0));
  color = pow(color, vec3(1.0 / 2.2));

  // shadowing
  float shadow_factor = CalcShadow();
  FragColor = vec4(color * shadow_factor, 1.0);
}