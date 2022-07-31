#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "base/debug.h"

namespace renderer {
struct Ray {
  glm::vec3 position;
  glm::vec3 direction;
  std::string AsString() const;
};

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

  bool operator==(const AABB& other) const = default;
#if CGDEBUG
  bool DebugCheckValid() const;
  bool DebugCheckNotNull() const;
  void SetColor(int level);
  void SetColor(const glm::vec4& color) { debug_color = color; }
  glm::vec4 debug_color = glm::vec4(0 ,0 ,0, 1);
#endif
};

struct AABBGPU {
  AABBGPU() = default;
  AABBGPU(const AABB& aabb);
  glm::vec4 maximum;
  glm::vec4 minimum;
};

template<typename ContainerType>
AABB UnionAABB(const std::vector<ContainerType>& containers, int begin, int end) {
  CGCHECK(end - begin > 0) << " Union AABB size < 1";
  CGCHECK(begin < containers.size()) << " begin index error";
  CGCHECK(end <= containers.size()) << " end index error";
  AABB res;
  for (int i = begin; i < end; ++i) {
    res.Union(containers[i].GetAABB());
  }
  return res;
}

template<typename ContainerType>
AABB UnionAABB(const std::vector<ContainerType>& containers, const std::vector<int>& indices) {
  AABB res;
  for (const int index : indices) {
    CGCHECK(index < containers.size()) << index << " " << containers.size();
    res.Union(containers[index].GetAABB());
  }
  return res;
}

struct RayAABBResult {
  bool hitted;
};
RayAABBResult RayAABB(const Ray& ray, const AABB& aabb);

struct Triangle {
  glm::vec3 a;
  glm::vec3 b;
  glm::vec3 c;
  AABB AsAABB() const;
  std::string AsString() const;
  glm::vec3 Normal() const;
  float GetArea() const;

  bool operator==(const Triangle& other) const = default;
};

struct TriangleGPU {
  TriangleGPU() = default;
  TriangleGPU(const Triangle& triangle, int material_index);
  glm::vec4 a;
  glm::vec4 b;
  glm::vec4 c;
};

struct RayTriangleResult {
  bool hitted;
  glm::vec3 position;
  glm::vec3 normal;
  float distance;
};

RayTriangleResult RayTriangle(const Ray& ray, const Triangle& triangle);

} // namespace renderer