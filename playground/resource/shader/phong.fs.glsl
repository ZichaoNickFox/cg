#version 330 core

out vec4 FragColor;

struct Material {
  bool use_texture_normal;
  bool use_texture_ambient;
  bool use_texture_specular;
  bool use_texture_diffuse;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  sampler2D texture_normal0;
  sampler2D texture_ambient0;
  sampler2D texture_specular0;
  sampler2D texture_diffuse0;

  float shininess;
};
uniform Material material;

struct Light {
  vec3 color;
  vec3 pos;
  float constant;   // attenuation
  float linear;     // attenuation
  float quadratic;  // attenuation
};
uniform int light_count;
uniform Light lights[200];

uniform vec3 view_pos;
uniform bool blinn_phong;

in mat3 world_TBN_;
in vec2 texcoord_;
in vec3 frag_world_pos_;
in vec3 frag_world_normal_;
in mat4 model_;

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

  // ambient
  vec3 ambient_component = light.color * ambient;

  // diffuse
  vec3 frag_2_light = normalize(light.pos - frag_world_pos_);
  float diff_factor = max(dot(normal, frag_2_light), 0.0);
  vec3 diffuse_component = light.color * diff_factor * diffuse;

  // specular
  vec3 frag_2_view = normalize(view_pos - frag_world_pos_);
  vec3 reflect_dir = reflect(-frag_2_light, normal);
  vec3 half_dir = normalize(frag_2_light + frag_2_view);
  float spec_factor = 0.0;
  if (blinn_phong) {
    spec_factor = pow(max(dot(normal, half_dir), 0.0), material.shininess * 2.0);
  } else {
    spec_factor = pow(max(dot(frag_2_view, reflect_dir), 0.0), material.shininess);
  }
  vec3 specular_component = light.color * spec_factor * specular;

  float dist = length(light.pos - frag_world_pos_);
  float attenuation = 1.0 / (dist * dist * light.quadratic + dist * light.linear + light.constant);

  vec3 result = (ambient_component + diffuse_component + specular_component) * attenuation;
  return result;
}

void main()
{
  // lighting
  vec3 light_result = vec3(0.0, 0.0, 0.0);
  for (int i = 0; i < light_count; ++i) {
    light_result += CalcLight(lights[i]);
  }
  FragColor = vec4(light_result, 0.5);
}