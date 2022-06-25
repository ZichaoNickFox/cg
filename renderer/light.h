#pragma once

#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>

#include "renderer/definition.h"
#include "renderer/ssbo.h"

namespace renderer {

/*
struct LightAttenuation {
  float constant;
  float linear;
  float quadratic;
};
static const std::unordered_map<uint32_t, LightAttenuation> kLightAttenuation = {
  {7, {1.0, 0.7, 1.8}},
  {13, {1.0, 0.35, 0.44}},
  {20, {1.0, 0.22, 0.2}},
  {32, {1.0, 0.14, 0.07}},
  {50, {1.0, 0.09, 0.032}},
  {65, {1.0, 0.07, 0.017}},
  {100, {1.0, 0.045, 0.0075}},
  {160, {1.0, 0.027, 0.0028}},
  {200, {1.0, 0.022, 0.0019}},
  {325, {1.0, 0.014, 0.0007}},
  {600, {1.0, 0.007, 0.0002}},
  {3250, {1.0, 0.0014, 0.000007}},
};
*/
struct Light {
  enum Type {
    kDirectionalLight = 0,
    kPointLight,
    kSpotLight
  };
  Type type;
  glm::vec3 pos;
  glm::vec4 color;
  glm::vec3 attenuation_2_1_0;

  bool operator==(const Light& other) const {
    return memcmp(this, &other, sizeof(Light)) == 0;
  }
};

struct LightRepo {
  LightRepo() : ssbo_(SSBO_LIGHT_REPO) {}
  void Add(const Light& light) { lights_.push_back(light); }
  void Add(const std::vector<Light>& lights) { lights_.insert(lights_.end(), lights.begin(), lights.end()); }
  void UpdateSSBO();
  int length() const { return lights_.size(); }
  std::vector<Light> GetLights() const { return lights_; };
  int light_num() const { return lights_.size(); }
  Light* mutable_light(int index) { return &lights_[index]; }
  const Light& light(int index) const { return lights_[index]; }

 private:
  struct LightGPU {
    glm::vec4 type_pos;
    glm::vec4 color;
    glm::vec4 attenuation_2_1_0;
  };
  std::vector<LightGPU> GetSSBOData();

  SSBO ssbo_;
  std::vector<Light> lights_;
  std::vector<Light> dirty_lights_;
};

} // namespace renderer
