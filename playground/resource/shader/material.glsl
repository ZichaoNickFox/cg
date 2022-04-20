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