struct PbrMaterial {
  vec3 albedo;
  float metallic;
  float roughness;
  vec3 ao;
  bool use_texture_normal;
  bool use_texture_albedo;
  bool use_texture_roughness;
  bool use_texture_metallic;
  bool use_texture_ao;
  sampler2D texture_normal0;
  sampler2D texture_albedo0;
  sampler2D texture_roughness0;
  sampler2D texture_metallic0;
  sampler2D texture_ao0;
};