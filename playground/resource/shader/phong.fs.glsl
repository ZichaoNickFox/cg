out vec4 FragColor;

uniform Material material;

uniform int light_count;
uniform Light lights[200];

uniform vec3 view_pos;
uniform bool blinn_phong;

uniform ShadowInfo shadow_info;
uniform bool use_shadowing = false;
in vec4 frag_shadow_pos_;

in mat3 world_TBN_;
in vec2 texcoord_;
in vec3 frag_world_pos_;
in vec3 frag_world_normal_;

vec3 CalcLight(Light light) {
  vec3 ambient = vec3(0, 0, 0);
  if (material.use_texture_ambient) {
    ambient = texture(material.texture_ambient0, texcoord_).xyz;
  } else {
    ambient = material.ambient;
  }

  vec3 normal = vec3(0, 0, 0);
  if (material.use_texture_normal) {
    vec3 normal_from_texture = texture(material.texture_normal0, texcoord_).xyz;
    normal_from_texture = normalize(normal_from_texture * 2.0 - 1.0);
    normal = normalize(world_TBN_ * normal_from_texture);
  } else {
    normal = normalize(frag_world_normal_);
  }

  vec3 diffuse = vec3(0.0);
  if (material.use_texture_diffuse) {
    diffuse = texture(material.texture_diffuse0, texcoord_).xyz;
  } else {
    diffuse = material.diffuse;
  }

  vec3 specular = vec3(0.0);
  if (material.use_texture_specular) {
    specular = texture(material.texture_specular0, texcoord_).xyz;
  } else {
    specular = material.specular;
  }

  PhongModelInput phong_model_input;
  phong_model_input.light_color = light.color;
  phong_model_input.light_pos_ws = light.pos;
  phong_model_input.light_quadratic = light.quadratic;
  phong_model_input.light_linear = light.linear;
  phong_model_input.light_constant = light.constant;
  phong_model_input.ambient = ambient;
  phong_model_input.diffuse = diffuse;
  phong_model_input.normal = normal;
  phong_model_input.specular = specular;
  phong_model_input.shininess = material.shininess;
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
  vec3 light_result = vec3(0.0, 0.0, 0.0);
  for (int i = 0; i < light_count; ++i) {
    light_result += CalcLight(lights[i]);
  }

  // shadowing
  float shadow_factor = CalcShadow();
  FragColor = vec4(light_result * shadow_factor, 1.0);
}