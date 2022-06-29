#include "renderer/shader/definition.glsl"
#include "renderer/shader/texture.glsl"

struct Material {
  vec4 albedo;
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  vec4 emission;
  vec4 roughness_metalness_shininess; // free w

::vec4 texture_index_normal_specular_ambient_diffuse;
::vec4 texture_index_basecolor_roughness_metalness_ambientocclusion;
::vec4 texture_index_height_shininess; // free y w
};

uniform int material_repo_num;
layout (std430, binding = SSBO_MATERIAL_REPO) buffer MaterialRepo {
  Material material_repo[];
};

vec4 MaterialAmbient(Material material, vec2 uv) {
  vec4 res = vec4(0, 0, 0, 1);
  int texture_ambient_index = int(material.texture_index_normal_specular_ambient_diffuse.z);
  if (texture_ambient_index != -1) {
    vec3 uvz = vec3(uv, texture_ambient_index);
    res = texture(texture_repo, uvz);
  } else {
    res = material.ambient;
  }
  return res;
}

vec3 MaterialNormal(Material material, mat3 world_TBN_, vec3 frag_world_normal, vec2 uv) {
  vec3 res = vec3(0, 0, 0);
  int texture_normal_index = int(material.texture_index_normal_specular_ambient_diffuse.x);
  if (texture_normal_index != -1) {
    vec3 uvz = vec3(uv, texture_normal_index);
    vec3 normal_from_texture = texture(texture_repo, uvz).xyz;
    normal_from_texture = normalize(normal_from_texture * 2.0 - 1.0);
    res = normalize(world_TBN_ * normal_from_texture).xyz;
  } else {
    res = normalize(frag_world_normal).xyz;
  }
  return res;
}

vec4 MaterialDiffuse(Material material) {
  vec4 res = vec4(0, 0, 0, 1);
  res = material.diffuse;
  return res;
}

vec4 MaterialDiffuse(Material material, vec2 uv) {
  vec4 res = vec4(0, 0, 0, 1);
  int texture_diffuse_index = int(material.texture_index_normal_specular_ambient_diffuse.w);
  if (texture_diffuse_index != -1) {
    vec3 uvz = vec3(uv, texture_diffuse_index);
    res = texture(texture_repo, uvz);
  } else {
    res = material.diffuse;
  }
  return res;
}

vec4 MaterialSpecular(Material material, vec2 uv) {
  vec4 res = vec4(0.0);
  int texture_specular_index = int(material.texture_index_normal_specular_ambient_diffuse.y);
  if (texture_specular_index != -1) {
    vec3 uvz = vec3(uv, texture_specular_index);
    res = texture(texture_repo, uvz);
  } else {
    res = material.specular;
  }
  return res;
}

vec4 MaterialEmission(Material material) {
  return material.emission;
}

float MaterialShininess(Material material, vec2 uv) {
  float res = 0;
  int texture_shininess_index = int(material.texture_index_height_shininess.y);
  if (texture_shininess_index != -1) {
    vec3 uvz = vec3(uv, texture_shininess_index);
    res = texture(texture_repo, uvz).x;
  } else {
    res = material.roughness_metalness_shininess.z;
  }
  return res;
}