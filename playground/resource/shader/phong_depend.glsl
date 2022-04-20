#version 330 core

struct ShadowInfo {
  mat4 light_vp;
  sampler2D texture_depth;
};

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

struct Light {
  vec3 color;
  vec3 pos;
  float constant;   // attenuation
  float linear;     // attenuation
  float quadratic;  // attenuation
};