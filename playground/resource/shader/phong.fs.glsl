out vec4 FragColor;

uniform PhongMaterial phong_material;

uniform vec3 view_pos;
uniform bool blinn_phong;

uniform ShadowInfo shadow_info;
uniform bool use_shadowing = false;
in vec4 frag_shadow_pos_;

in mat3 world_TBN_;
in vec2 texcoord_;
in vec3 frag_world_pos_;
in vec3 frag_world_normal_;

vec3 CalcLight() {
  vec3 ambient = vec3(0, 0, 0);
  if (phong_material.use_texture_ambient) {
    ambient = texture(phong_material.texture_ambient0, texcoord_).xyz;
  } else {
    ambient = phong_material.ambient;
  }

  vec3 normal = vec3(0, 0, 0);
  if (phong_material.use_texture_normal) {
    vec3 normal_from_texture = texture(phong_material.texture_normal0, texcoord_).xyz;
    normal_from_texture = normalize(normal_from_texture * 2.0 - 1.0);
    normal = normalize(world_TBN_ * normal_from_texture);
  } else {
    normal = normalize(frag_world_normal_);
  }

  vec3 diffuse = vec3(0.0);
  if (phong_material.use_texture_diffuse) {
    diffuse = texture(phong_material.texture_diffuse0, texcoord_).xyz;
  } else {
    diffuse = phong_material.diffuse;
  }

  vec3 specular = vec3(0.0);
  if (phong_material.use_texture_specular) {
    specular = texture(phong_material.texture_specular0, texcoord_).xyz;
  } else {
    specular = phong_material.specular;
  }

  PhongModelInput phong_model_input;
  phong_model_input.ambient = ambient;
  phong_model_input.diffuse = diffuse;
  phong_model_input.normal = normal;
  phong_model_input.specular = specular;
  phong_model_input.shininess = phong_material.shininess;
  phong_model_input.frag_pos_ws = frag_world_pos_;
  phong_model_input.view_pos_ws = view_pos;
  phong_model_input.blinn_phong = blinn_phong;
  return PhongModel(phong_model_input);
}

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
  // lighting
  vec3 light_result = CalcLight();

  // shadowing
  float shadow_factor = CalcShadow();
  FragColor = vec4(light_result * shadow_factor, 1.0);
}