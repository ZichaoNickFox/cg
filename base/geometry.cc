#include "base/geometry.h"

#include <glm/gtx/quaternion.hpp>
#include "glm/gtx/string_cast.hpp"

#include "base/color.h"
#include "base/debug.h"
#include "base/math.h"
#include "base/util.h"

namespace cg {
std::string Ray::AsString() const {
  std::string res;
  res += "origin ~ " + glm::to_string(position);
  res += "dir ~ " + glm::to_string(direction);
  return res;
}

glm::vec3 GetFootOfPerpendicular(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b) {
  glm::vec3 ap = p - a;
  glm::vec3 ab = b - a;
  float ab_length2 = glm::dot(ab, ab);
  if (ab_length2 <= std::numeric_limits<float>::epsilon()) {
    return a;
  }
  glm::vec3 foot = a + glm::dot(ap, ab) / ab_length2 * ab;
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
  return 2 * (delta.x * delta.y + delta.y * delta.z + delta.x * delta.z);
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
  return util::Format("{},{},{}|{},{},{}",
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
  RayAABBResult res = {.hitted = false};

  auto update_interval = [](float origin, float direction, float minimum, float maximum, float* t_enter,
                            float* t_exit) {
    if (std::abs(direction) <= std::numeric_limits<float>::epsilon()) {
      return origin >= minimum && origin <= maximum;
    }

    float axis_enter = (minimum - origin) / direction;
    float axis_exit = (maximum - origin) / direction;
    if (axis_enter > axis_exit) {
      std::swap(axis_enter, axis_exit);
    }
    *t_enter = std::max(*t_enter, axis_enter);
    *t_exit = std::min(*t_exit, axis_exit);
    return *t_enter <= *t_exit;
  };

  float t_enter = std::numeric_limits<float>::lowest();
  float t_exit = std::numeric_limits<float>::max();
  if (!update_interval(ray.position.x, ray.direction.x, aabb.minimum.x, aabb.maximum.x, &t_enter, &t_exit) ||
      !update_interval(ray.position.y, ray.direction.y, aabb.minimum.y, aabb.maximum.y, &t_enter, &t_exit) ||
      !update_interval(ray.position.z, ray.direction.z, aabb.minimum.z, aabb.maximum.z, &t_enter, &t_exit)) {
    return res;
  }

  res.hitted = (t_enter <= t_exit && t_exit >= 0);
  return res;
}

std::string Triangle::AsString() const {
  std::string res;
  res += "a ~ " + glm::to_string(a);
  res += "b ~ " + glm::to_string(b);
  res += "c ~ " + glm::to_string(c);
  return res;
}

glm::vec3 Triangle::Normal() const {
  return glm::cross(b - a, c - a);
}

float Triangle::GetArea() const {
  glm::vec3 e1 = b - a;
  glm::vec3 e2 = c - a;
  return glm::length(glm::cross(e1, e2)) / 2.0f;
}

// Mollar Trumbore Algorithm
// wikipedia https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
// GAMES101 https://www.bilibili.com/video/BV1X7411F744?p=13 0:51:50
RayTriangleResult RayTriangle(const Ray& ray, const Triangle& triangle) {
  RayTriangleResult res;
  res.hitted = false;

  glm::vec3 edge1, edge2, h, s, q;
  float a,f,u,v;

  edge1 = triangle.b - triangle.a;
  edge2 = triangle.c - triangle.a;
  h = glm::cross(ray.direction, edge2);
  a = glm::dot(edge1, h);
  if (std::abs(a) < std::numeric_limits<float>::epsilon()) {
    return res; // The ray is parallel to the triangle
  }
  f = 1.0 / a;
  s = ray.position - triangle.a;
  u = f * glm::dot(s, h);
  if (u < 0.0 || u > 1.0)
    return res;
  q = glm::cross(s, edge1);
  v = f * glm::dot(ray.direction, q);
  if (v < 0.0 || u + v > 1.0) {
    return res;
  }
  float t = f * glm::dot(edge2, q); // Find out where the intersection point is on the line
  if (t < 0) {
    return res;
  }

  res.hitted = true;
  res.distance = t;
  res.normal = glm::normalize(glm::cross(edge1, edge2));
  res.position = ray.position + ray.direction * t;
  return res;
}

TriangleGPU::TriangleGPU(const Triangle& triangle, int material_index) {
  a = glm::vec4(triangle.a, 0.0);
  b = glm::vec4(triangle.b, 0.0);
  c = glm::vec4(triangle.c, 0.0);
}

void LineSegment::Bresenham(int width, int height, const std::function<void(const glm::vec2&)>& callback) const {
  float dx = b.x - a.x;
  float dy = b.y - a.y;
  float xsign = dx > 0 ? 1 : -1;
  float ysign = dy > 0 ? 1 : -1;
  dx = std::abs(dx);
  dy = std::abs(dy);
  float xx, xy, yx, yy;
  if (dx > dy) {
    xx = xsign, xy = 0, yx = 0, yy = ysign;
  } else {
    std::swap(dx, dy);
    xx = 0, xy = ysign, yx = xsign, yy = 0;
  }
  float D = 2 * dy - dx;
  float y = 0;
  std::vector<glm::vec2> res;
  for (float x = 0; x < dx + 1; x += 1) {
    glm::vec2 v(math::Clamp(a.x + x * xx, 0, width - 1), math::Clamp(a.y + x * xy + y * yy, 0, height - 1));
    callback(v);
    if (D >= 0) {
      y += 1;
      D -= 2 * dx;
    }
    D += 2 * dy;
  }
}

} // namespace cg
