#pragma once

#include <map>
#include <unordered_map>

namespace engine {
struct LightAttenuation {
  float constant;
  float linear;
  float quadratic;
};
static const std::map<uint32_t, LightAttenuation> kLightAttenuation = {
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

struct MaterialProperty {
  glm::vec4 ambient;
  glm::vec4 diffuse;
  glm::vec4 specular;
  float shininess;
};
static const std::unordered_map<std::string, MaterialProperty> kMaterialProperties = {
  {"gold", {glm::vec4(0.24725, 0.1995, 0.0745, 1), glm::vec4(0.75164, 0.60648, 0.22648, 1),
            glm::vec4(0.628281, 0.555802, 0.366065, 1), 51.2}},
  {"silver", {glm::vec4(0.19225, 0.19225, 0.19225, 1), glm::vec4(0.50754, 0.50754, 0.50754, 1),
              glm::vec4(0.508273, 0.508273, 0.508273, 1), 51.2}},
  {"jade", {glm::vec4(0.135, 0.2225, 0.1575, 0.95), glm::vec4(0.54, 0.89, 0.63, 0.95),
            glm::vec4(0.316228, 0.316228, 0.316228, 0.95), 12.8}},
  {"ruby", {glm::vec4(0.1745, 0.01175, 0.01175, 0.55), glm::vec4(0.61424, 0.04136, 0.04136, 0.55),
            glm::vec4(0.727811, 0.626959, 0.626959, 0.55), 76.8}},
};
} // namespace engine
