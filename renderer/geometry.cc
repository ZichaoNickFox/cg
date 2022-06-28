#include "renderer/geometry.h"

#include <glm/gtx/quaternion.hpp>
#include "glm/gtx/string_cast.hpp"

#include "renderer/color.h"
#include "renderer/debug.h"
#include "renderer/util.h"

namespace renderer {
std::string Ray::AsString() const {
  std::string res;
  res += "origin ~ " + glm::to_string(origin);
  res += "dir ~ " + glm::to_string(dir);
  return res;
}

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

AABBGPU::AABBGPU(const AABB& aabb) {
  maximum = glm::vec4(aabb.maximum, 0.0);
  minimum = glm::vec4(aabb.minimum, 0.0);
}

AABB Triangle::AsAABB() const {
  float maximum_x = std::max(std::max(a.x, b.x), c.x);
  float maximum_y = std::max(std::max(a.y, b.y), c.y);
  float maximum_z = std::max(std::max(a.z, b.z), c.z);
  float minimum_x = std::min(std::min(a.x, b.x), c.x);
  float minimum_y = std::min(std::min(a.y, b.y), c.y);
  float minimum_z = std::min(std::min(a.z, b.z), c.z);
  float epsilon = std::numeric_limits<float>::epsilon();
  float epsilon10 = epsilon * 10;
  if (maximum_z - minimum_z < epsilon) {
    maximum_z += epsilon10;
  }
  if (maximum_y - minimum_y < epsilon) {
    maximum_y += epsilon10;
  }
  if (maximum_x - minimum_x < epsilon) {
    maximum_x += epsilon10;
  }
  return {{maximum_x, maximum_y, maximum_z}, {minimum_x, minimum_y, minimum_z}};
}

std::string AABB::AsString() const {
  return fmt::format("{},{},{}|{},{},{}",
                      maximum.x, maximum.y, maximum.z, minimum.x, minimum.y, minimum.z);
}

#if CGDEBUG
bool AABB::DebugCheckValid() const {
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
  return true;
}

bool AABB::DebugCheckNotNull() const {
  CGCHECK(maximum.x != std::numeric_limits<float>::lowest() && maximum.y != std::numeric_limits<float>::lowest()
    && maximum.z != std::numeric_limits<float>::lowest());
  CGCHECK(minimum.x != std::numeric_limits<float>::max() && minimum.y != std::numeric_limits<float>::max()
    && minimum.z != std::numeric_limits<float>::max());
  return true;
}

void AABB::SetColor(int level) {
  if (level == 0) { debug_color = kRed; }
  else if (level == 1) { debug_color = kOrange; }
  else if (level == 2) { debug_color = kYellow; }
  else if (level == 3) { debug_color = kGreen; }
  else if (level == 4) { debug_color = kCyan; }
  else if (level == 5) { debug_color = kBlue; }
  else if (level == 6) { debug_color = kPurple; }
  else if (level == 7) { debug_color = kBlack; }
  else if (level == 8) { debug_color = kWhite; }
  else { debug_color = kGray; }
}
#endif

RayAABBResult RayAABB(const Ray& ray, const AABB& aabb) {
  RayAABBResult res;

  float t_enter_x = std::numeric_limits<float>::lowest();
  float t_enter_y = std::numeric_limits<float>::lowest();
  float t_enter_z = std::numeric_limits<float>::lowest();
  float t_exit_x = std::numeric_limits<float>::max();
  float t_exit_y = std::numeric_limits<float>::max();
  float t_exit_z = std::numeric_limits<float>::max();
  if (ray.dir.x > 0) {
    t_enter_x = (aabb.minimum.x - ray.origin.x) / ray.dir.x;
    t_exit_x = (aabb.maximum.x - ray.origin.x) / ray.dir.x;
  } else if (ray.dir.x < 0) {
    t_enter_x = (aabb.maximum.x - ray.origin.x) / ray.dir.x;
    t_exit_x = (aabb.minimum.x - ray.origin.x) / ray.dir.x;
  }

  if (ray.dir.y > 0) {
    t_enter_y = (aabb.minimum.y - ray.origin.y) / ray.dir.y;
    t_exit_y = (aabb.maximum.y - ray.origin.y) / ray.dir.y;
  } else if (ray.dir.y < 0) {
    t_enter_y = (aabb.maximum.y - ray.origin.y) / ray.dir.y;
    t_exit_y = (aabb.minimum.y - ray.origin.y) / ray.dir.y;
  }

  if (ray.dir.z > 0) {
    t_enter_z = (aabb.minimum.z - ray.origin.z) / ray.dir.z;
    t_exit_z = (aabb.maximum.z - ray.origin.z) / ray.dir.z;
  } else if (ray.dir.z < 0) {
    t_enter_z = (aabb.maximum.z - ray.origin.z) / ray.dir.z;
    t_exit_z = (aabb.minimum.z - ray.origin.z) / ray.dir.z;
  }

  float t_enter = std::max(std::max(t_enter_x, t_enter_y), t_enter_z);
  float t_exit = std::min(std::min(t_exit_x, t_exit_y), t_exit_z);

  res.hitted = (t_enter < t_exit && t_exit >= 0);

  return res;
}

std::string Triangle::AsString() const {
  std::string res;
  res += "a ~ " + glm::to_string(a);
  res += "b ~ " + glm::to_string(b);
  res += "c ~ " + glm::to_string(c);
  return res;
}

glm::vec3 Triangle::normal() const {
  return glm::cross(b - a, c - a);
}

// Mollar Trumbore Algorithm
// wikipedia
// https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
// GAMES101
// https://www.bilibili.com/video/BV1X7411F744?p=13 0:51:50
RayTriangleResult RayTriangle(const Ray& ray, const Triangle& triangle) {
  RayTriangleResult res;
  res.hitted = false;

  glm::vec3 edge1, edge2, h, s, q;
  float a,f,u,v;

  edge1 = triangle.b - triangle.a;
  edge2 = triangle.c - triangle.a;
  h = glm::cross(ray.dir, edge2);
  a = glm::dot(edge1, h);
  if (std::abs(a) < std::numeric_limits<float>::epsilon()) {
    return res; // The ray is parallel to the triangle
  }
  f = 1.0 / a;
  s = ray.origin - triangle.a;
  u = f * glm::dot(s, h);
  if (u < 0.0 || u > 1.0)
    return res;
  q = glm::cross(s, edge1);
  v = f * glm::dot(ray.dir, q);
  if (v < 0.0 || u + v > 1.0) {
    return res;
  }
  float t = f * glm::dot(edge2, q); // Find out where the intersection point is on the line
  if (t < 0) {
    return res;
  }

  res.hitted = true;
  res.dist = t;
  res.normal = glm::normalize(glm::cross(edge1, edge2));
  res.pos = ray.origin + ray.dir * t;
  return res;
}

TriangleGPU::TriangleGPU(const Triangle& triangle) {
  a = glm::vec4(triangle.a, 0.0);
  b = glm::vec4(triangle.b, 0.0);
  c = glm::vec4(triangle.c, 0.0);
}
} // namespace renderer