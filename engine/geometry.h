#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace engine {
struct Sphere {
  int id;
  glm::vec3 translation;
  glm::vec4 color;
  float radius;
};

glm::vec3 GetFootOfPerpendicular(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b);

struct AABB {
  enum Axis { kX, kY, kZ };

  glm::vec3 maximum = glm::vec3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(),
                                std::numeric_limits<float>::min());
  glm::vec3 minimum = glm::vec3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                                std::numeric_limits<float>::max());
  void Union(const AABB& other);
  float SurfaceArea() const;
  glm::vec3 Center() const;
  Axis GetMaxLengthAxis() const;
  float GetLengthByAxis(Axis axis) const;
  float GetMinimumByAxis(Axis axis) const;
  float GetMaximumByAxis(Axis axis) const;
  float GetCenterByAxis(Axis axis) const;
};

AABB UnionAABB(const std::vector<AABB>& aabbs, int begin, int end);

struct Triangle {
  glm::vec3 a;
  glm::vec3 b;
  glm::vec3 c;
  AABB AsAABB();
};
} // namespace engine