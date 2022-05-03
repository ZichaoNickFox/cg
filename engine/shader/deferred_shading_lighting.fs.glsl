#version 410 core

out vec4 FragColor;
in vec2 texcoord_;

struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};
uniform Material material;

struct Light {
  vec3 color;
  vec3 pos;

  // attenuation
  float constant;
  float linear;
  float quadratic;
};
uniform int light_count;
uniform Light lights[200];

uniform vec3 view_pos;

uniform sampler2D texture_normal;
uniform sampler2D texture_frag_world_pos;
uniform vec3 clear_color;

// shadow map
uniform sampler2D shadow_map_texture;
in vec4 shadow_map_space_pos_;

vec3 CalcLight(Light light, vec3 normal_, vec3 frag_world_pos_) {
  // ambient
  vec3 ambient = light.color * material.ambient;

  // diffuse
  vec3 norm = normalize(normal_);
  vec3 light_dir = normalize(light.pos - frag_world_pos_);
  float diff_factor = max(dot(norm, light_dir), 0.0);
  vec3 diffuse = light.color * diff_factor * material.diffuse;

  // specular
  vec3 view_dir = normalize(view_pos - frag_world_pos_);
  vec3 reflect_dir = reflect(-light_dir, norm);
  float spec_factor = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
  vec3 specular = light.color * spec_factor * material.specular;

  float dist = length(light.pos - frag_world_pos_);
  float attenuation = 1.0 / (dist * dist * light.quadratic + dist * light.linear + light.constant);

  vec3 result = (ambient + diffuse + specular) * attenuation;
  return result;
}

void main()
{
  // lighting
  vec3 light_result = vec3(0, 0, 0);
  vec3 normal_ = texture(texture_normal, texcoord_).xyz;
  vec3 frag_world_pos_ = texture(texture_frag_world_pos, texcoord_).xyz;

  if (frag_world_pos_ == clear_color) {
    FragColor = vec4(clear_color, 1.0);
    return;
  }
  for (int i = 0; i < light_count; ++i) {
    light_result += CalcLight(lights[i], normal_, frag_world_pos_);
  }

  // shadow_map
  // https://zhuanlan.zhihu.com/p/65969162
  // / w : clip space -> NDC space
  // https://forum.unity.com/threads/confused-on-ndc-space.1024414/
  // * 0.5 + 0.5 depth : NDC space -> screen space

  /*
  vec4 shadow_map_space_pos_ndc = shadow_map_space_pos_ / shadow_map_space_pos_.w;
  shadow_map_space_pos_ndc = shadow_map_space_pos_ndc * 0.5 + 0.5;

  float shadow_map_sample = texture(shadow_map_texture, shadow_map_space_pos_ndc.xy).r;
  bool in_shadow = shadow_map_sample + 0.001 < shadow_map_space_pos_ndc.z;
  float shadow_factor = in_shadow ? 0.5 : 1;

  FragColor = vec4(light_result * shadow_factor, 1.0);
  */
  FragColor = vec4(light_result, 1.0);
}