#include "engine/geometry.h"

glm::vec3 GetFootOfPerpendicular(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b) {
  glm::vec3 ap = p - a;
  glm::vec3 ab = b - a;
  glm::vec3 foot = a + glm::dot(ap, ab) / glm::length(ab);
}