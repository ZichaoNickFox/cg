#pragma once

#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>

#include "renderer/definition.h"
#include "renderer/material.h"
#include "renderer/primitive.h"
#include "renderer/ssbo.h"

namespace renderer {

// Light Attenuation
// {7, {1.0, 0.7, 1.8}},
// {65, {1.0, 0.07, 0.017}},
// {600, {1.0, 0.007, 0.0002}},
// {3250, {1.0, 0.0014, 0.000007}},
struct Light {
  enum Type {
    kPrimitiveLight = 0,
    kDirectionalLight,
    kPointLight,
    kSpotLight
  };
  Light(Type in_type, int in_primitive_id);
  Light(Type in_type, const glm::vec3& in_position, const glm::vec4& in_color, const glm::vec3& in_attenuation_2_1_0);
  Type type;
  int primitive_index = -1;
  glm::vec3 position;
  glm::vec4 color;
  glm::vec3 attenuation_2_1_0;

  bool operator==(const Light& other) const = default;
};

struct LightRepo {
  LightRepo() : ssbo_(SSBO_LIGHT_REPO) {}
  void Add(const std::vector<Light>& lights) { lights_.insert(lights_.end(), lights.begin(), lights.end()); }
  void Add(const PrimitiveRepo& primitive_repo, const MaterialRepo& material_repo); 
  void UpdateSSBO();
  std::vector<Light> GetLights() const { return lights_; };
  int num() const { return lights_.size(); }
  Light* mutable_light(int index) { return &lights_[index]; }
  const Light& light(int index) const { return lights_[index]; }

  int primitive_light_num() const { return primitive_light_num_; }
  float primitive_light_area() const { return primitive_light_area_; }

 private:
  struct LightGPU {
    LightGPU() = default;
    LightGPU(const Light& light);
    glm::vec4 type_primitiveindex;
    glm::vec4 position;
    glm::vec4 color;
    glm::vec4 attenuation_2_1_0;
  };
  SSBO ssbo_;
  std::vector<Light> lights_;

  // Primitive Light
  int primitive_light_num_;
  float primitive_light_area_;
  std::vector<Light> dirty_lights_;
};

} // namespace renderer
