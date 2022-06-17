#include "engine/geometry.h"

#include "glm/gtx/string_cast.hpp"

#include "engine/debug.h"
#include "engine/util.h"

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
  } else {
    CGCHECK(false) << "Logic Error";
    return 0;
  }
}

float AABB::GetCenterByAxis(Axis axis) const {
  glm::vec3 center = Center();
  if (axis == Axis::kX) {
    return center.x;
  } else if (axis == Axis::kY) {
    return center.y;
  } else if (axis == Axis::kZ) {
    return center.z;
  } else {
    CGCHECK(false) << "Logic Error";
    return 0;
  }
}

float AABB::GetMinimumByAxis(Axis axis) const {
  if (axis == Axis::kX) {
    return minimum.x;
  } else if (axis == Axis::kY) {
    return minimum.y;
  } else if (axis == Axis::kZ) {
    return minimum.z;
  } else {
    CGCHECK(false) << "Logic Error";
    return 0;
  }
}

float AABB::GetMaximumByAxis(Axis axis) const {
  if (axis == Axis::kX) {
    return maximum.x;
  } else if (axis == Axis::kY) {
    return maximum.y;
  } else if (axis == Axis::kZ) {
    return maximum.z;
  } else {
    CGCHECK(false) << "Logic Error";
    return 0;
  }
}

AABB Triangle::AsAABB() {
  float maximum_x = std::max(std::max(a.x, b.x), c.x);
  float maximum_y = std::max(std::max(a.y, b.y), c.y);
  float maximum_z = std::max(std::max(a.z, b.z), c.z);
  float minimum_x = std::min(std::min(a.x, b.x), c.x);
  float minimum_y = std::min(std::min(a.y, b.y), c.y);
  float minimum_z = std::min(std::min(a.z, b.z), c.z);
  return {{maximum_x, maximum_y, maximum_z}, {minimum_x, minimum_y, minimum_z}};
}

std::string AABB::AsString() const {
  return util::Format("{},{},{}|{},{},{}",
                      maximum.x, maximum.y, maximum.z, minimum.x, minimum.y, minimum.z);
}

bool AABB::CheckValid() const {
#ifdef DEBUG_SLOW
  if (maximum.x == std::numeric_limits<float>::lowest() || maximum.y == std::numeric_limits<float>::lowest()
      || maximum.z == std::numeric_limits<float>::lowest()) {
    CGCHECK(maximum.x == std::numeric_limits<float>::lowest() && maximum.y == std::numeric_limits<float>::lowest()
      && maximum.z == std::numeric_limits<float>::lowest());
  }
  if (minimum.x == std::numeric_limits<float>::max() || minimum.y == std::numeric_limits<float>::max()
      || minimum.z == std::numeric_limits<float>::max()) {
    CGCHECK(minimum.x == std::numeric_limits<float>::max() && minimum.y == std::numeric_limits<float>::max()
      && minimum.z == std::numeric_limits<float>::max());
  }
#endif
  return true;
}

bool AABB::CheckNotNull() const {
#ifdef DEBUG_SLOW
  CGCHECK(maximum.x != std::numeric_limits<float>::lowest() && maximum.y != std::numeric_limits<float>::lowest()
    && maximum.z != std::numeric_limits<float>::lowest());
  CGCHECK(minimum.x != std::numeric_limits<float>::max() && minimum.y != std::numeric_limits<float>::max()
    && minimum.z != std::numeric_limits<float>::max());
#endif
  return true;
}
} // namespace engine