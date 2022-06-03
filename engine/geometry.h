#pragma once

#include <glm/glm.hpp>

namespace engine {
struct SphereGeometry {
  glm::vec3 translation;
  glm::vec4 color;
  float radius;
};

glm::vec3 GetFootOfPerpendicular(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b);
}