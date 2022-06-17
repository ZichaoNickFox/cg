#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "engine/debug.h"

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

  glm::vec3 maximum = glm::vec3(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(),
                                std::numeric_limits<float>::lowest());
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

  std::string AsString() const;
  bool CheckValid() const;
  bool CheckNotNull() const;
};

template<typename ContainerType>
AABB UnionAABB(const std::vector<ContainerType>& containers, int begin, int end) {
  CGCHECK(end - begin > 0) << " Union AABB size < 1";
  CGCHECK(begin < containers.size()) << " begin index error";
  CGCHECK(end <= containers.size()) << " end index error";
  AABB res;
  for (const ContainerType& container : containers) {
    res.Union(container.GetAABB());
  }
  return res;
}

struct Triangle {
  glm::vec3 a;
  glm::vec3 b;
  glm::vec3 c;
  AABB AsAABB();
};
} // namespace engine