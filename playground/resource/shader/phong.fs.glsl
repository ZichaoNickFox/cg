#version 330 core

out vec4 FragColor;

struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};
uniform Material material;

uniform vec3 light_color;
uniform vec3 light_pos;
uniform vec3 view_pos;

in vec3 normal_;
in vec3 frag_pos_;

// shadow map
uniform sampler2D shadow_map_texture;
in vec4 shadow_map_space_pos_;

void main()
{
  // ambient
  vec3 ambient = light_color * material.ambient;

  // diffuse
  vec3 norm = normalize(normal_);
  vec3 light_dir = normalize(light_pos - frag_pos_);
  float diff_factor = max(dot(norm, light_dir), 0.0);
  vec3 diffuse = light_color * diff_factor * material.diffuse;

  // specular
  vec3 view_dir = normalize(view_pos - frag_pos_);
  vec3 reflect_dir = reflect(-light_dir, norm);
  float spec_factor = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
  vec3 specular = light_color * spec_factor * material.specular;

  vec3 result = ambient + diffuse + specular;

  // https://zhuanlan.zhihu.com/p/65969162
  // / w : clip space -> NDC space
  // https://forum.unity.com/threads/confused-on-ndc-space.1024414/
  // * 0.5 + 0.5 depth : NDC space -> screen space
  vec4 shadow_map_space_pos_ndc = shadow_map_space_pos_ / shadow_map_space_pos_.w;
  shadow_map_space_pos_ndc = shadow_map_space_pos_ndc * 0.5 + 0.5;

  float shadow_map_sample = texture(shadow_map_texture, shadow_map_space_pos_ndc.xy).r;
  bool in_shadow = shadow_map_sample + 0.001 < shadow_map_space_pos_ndc.z;
  float shadow_factor = in_shadow ? 0.5 : 1;
  FragColor = vec4(result * shadow_factor, 1.0);
}