#pragma once

#include "engine/material.h"
#include "playground/context.h"

struct MaterialLightInfo {
  std::vector<glm::vec3> light_poses;
  std::vector<glm::vec3> light_colors;
  // 7 / 13 / 20 / 32 / 50 / 65 / 100 / 160 / 200 / 325 / 600 / 3250
  std::vector<int> light_attenuation_metres;
  bool use_blinn_phong;
};

struct PhongMaterialPrefab {
  // gold / silver / jade / ruby
  std::string material_propery_name; 
  MaterialLightInfo material_light_info;
};

namespace material_prefab {

void UpdatePhongMaterial(Context* context, const PhongMaterialPrefab& phong, engine::Material* material);

};