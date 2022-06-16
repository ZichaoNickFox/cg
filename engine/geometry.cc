#include "engine/geometry.h"

#include "engine/debug.h"

namespace engine {
glm::vec3 GetFootOfPerpendicular(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b) {
  glm::vec3 ap = p - a;
  glm::vec3 ab = b - a;
  glm::vec3 foot = a + glm::dot(ap, ab) / glm::length(ab);
  return foot;
}

void AABB::Union(const AABB& other) {
  maximum.x = std::max(maximum.x, other.maximum.x);
  maximum.y = std::max(maximum.y, other.maximum.y);
  maximum.z = std::max(maximum.z, other.maximum.z);
  minimum.x = std::min(minimum.x, other.minimum.x);
  minimum.y = std::min(minimum.y, other.minimum.y);
  minimum.z = std::min(minimum.z, other.minimum.z);
}

float AABB::SurfaceArea() const {
  glm::vec3 delta = maximum - minimum;
  return 2 * (delta.x * delta.y + delta.y * delta.z * delta.x * delta.z);
}

glm::vec3 AABB::Center() const {
  return glm::vec3(0.5, 0.5, 0.5) * (maximum + minimum);
}

AABB::Axis AABB::GetMaxLengthAxis() const {
  glm::vec3 delta = maximum - minimum;
  if (delta.x >= delta.y && delta.x >= delta.z) {
    return kX;
  } else if (delta.y >= delta.x && delta.y >= delta.z) {
    return kY;
  } else if (delta.z >= delta.x && delta.z >= delta.y) {
    return kZ;
  } else {
    CGCHECK(false) << " Logic Error This Function ?";
    return kX;
  }
}

float AABB::GetLengthByAxis(Axis axis) const {
  glm::vec3 delta = maximum - minimum;
  if (axis == Axis::kX) {
    return delta.x;
  } else if (axis == Axis::kY) {
    return delta.y;
  } else if (axis == Axis::kZ) {
    return delta.z;
  }
}

float AABB::GetCenterByAxis(Axis axis) const {
  glm::vec3 center = Center();
  if (axis == Axis::kX) {
    return center.x;
  } else if (axis == Axis::kY) {
    return center.y;
  } else {
    return center.z;
  }
}

AABB UnionAABB(const std::vector<AABB>& aabbs, int begin, int end) {
  CGCHECK(end - begin > 0) << " Union AABB size < 1";
  CGCHECK(begin < aabbs.size()) << " begin index error";
  CGCHECK(end <= aabbs.size()) << " end index error";
  AABB res;
  for (const AABB& aabb : aabbs) {
    res.Union(aabb);
  }
  return res;
}

AABB Triangle::AsAABB() {
  float maximum_x = std::max(std::max(a.x, b.x), c.x);
  float maximum_y = std::max(std::max(a.y, b.y), c.y);
  float maximum_z = std::max(std::max(a.z, b.z), c.z);
  float minimum_x = std::min(std::min(a.x, b.x), c.x);
  float minimum_y = std::min(std::min(a.y, b.y), c.y);
  float minimum_z = std::min(std::min(a.z, b.z), c.z);
  return AABB{{maximum_x, maximum_y, maximum_z}, {minimum_x, minimum_y, minimum_z}};
}
} // namespace engine