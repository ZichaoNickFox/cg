#pragma once

#include "engine/material.h"
#include "playground/context.h"

struct ShaderLightInfo {
  std::vector<glm::vec3> light_poses;
  std::vector<glm::vec3> light_colors;
  std::vector<int> light_attenuation_metres; // 7 / 13 / 20 / 32 / 50 / 65 / 100 / 160 / 200 / 325 / 600 / 3250
};

struct PhongShaderParam {
  std::string shader_name = "phong";
  std::string material_propery_name; // gold / silver / jade / ruby
  ShaderLightInfo light_info;
  bool use_blinn_phong;
};

struct PbrShaderParam {
  std::string shader_name = "pbr";
  glm::vec3 albedo;
  float metallic;
  float roughness;
  float ao;
  ShaderLightInfo light_info;
};

namespace shader_param {

void UpdateShader(Context* context, const PhongShaderParam& phong, engine::Material* material);
void UpdateShader(Context* context, const PbrShaderParam& pbr, engine::Material* material);

};