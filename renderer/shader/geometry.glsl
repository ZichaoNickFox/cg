#include "renderer/shader/definition.glsl"

// Ray
// Sphere
// Plane
// AABB
// Triangle

// RaySphereResult
// RaySphere
// RayPlaneResult
// RayPlane
// RayAABBResult
// RayAABB
// RayTriangleResult
// RayTriangle

struct Ray {
  vec3 position;
  vec3 direction; // must normalized
};

struct Sphere {
  int id;
  vec3 center_position;
  vec4 color;
  float radius;
};

struct Plane {
  vec3 point;
  vec3 normal; // must normalized
};

struct AABB {
  vec4 maximum;
  vec4 minimum;
};

struct Triangle {
  vec4 a;
  vec4 b;
  vec4 c;
};

vec3 TriangleNormal(Triangle triangle) {
  return cross(triangle.b.xyz - triangle.a.xyz, triangle.c.xyz - triangle.a.xyz);
}

struct RaySphereResult {
  bool hitted;
  vec3 position;
  float distance;
  vec3 normal;
};

// https://www.bilibili.com/video/BV1X7411F744?p=13 - 0:31:19
RaySphereResult RaySphere(Sphere sphere, Ray ray, float limit) {
  RaySphereResult res;
  res.hitted = false;

  vec3 normalized_direction = normalize(ray.direction);
  vec3 oc = sphere.center_position - ray.position;
  float a = dot(normalized_direction, normalized_direction);
  float b = -2 * dot(normalized_direction, oc);
  float c = dot(oc, oc) - sphere.radius * sphere.radius;
  float delta = b * b - 4 * a * c;
  if (delta >= 0) {
    float sqrtd = sqrt(delta);
    float root1 = (-b - sqrtd) / (2 * a);
    float root2 = (-b + sqrtd) / (2 * a);
    if (root1 > 0 && root1 <= limit) {
      res.distance = root1;
      res.position = ray.position + res.distance * normalized_direction;
      res.hitted = true;
      res.normal = normalize(res.position - sphere.center_position);
    } else if (root2 > 0 && root2 <= limit) {
      res.distance = root2;
      res.position = ray.position + res.distance * normalized_direction;
      res.hitted = true;
      res.normal = normalize(res.position - sphere.center_position);
    }
  }
  return res;
}

struct RayPlaneResult {
  bool hitted;
  vec3 pos;
  float distance;
};

// https://www.bilibili.com/video/BV1X7411F744?p=13 - 1:14:45
RayPlaneResult RayPlane(Ray ray, Plane plane) {
  RayPlaneResult res;
  res.distance = (plane.point.x - ray.position.x) / ray.direction.x;
  res.hitted = res.distance >= 0.0;
  if (res.hitted) {
    res.pos = ray.position + ray.direction * res.distance;
  }
  return res;
}

struct RayTriangleResult {
  bool hitted;
  vec3 position;
  vec3 normal;
  float distance;
};

// https://www.bilibili.com/video/BV1X7411F744?p=13 0:51:50
RayTriangleResult RayTriangle(Ray ray, Triangle triangle) {
  RayTriangleResult res;
  res.hitted = false;

  vec3 edge1, edge2, h, s, q;
  float a,f,u,v;

  edge1 = triangle.b.xyz - triangle.a.xyz;
  edge2 = triangle.c.xyz - triangle.a.xyz;
  h = cross(ray.direction, edge2);
  a = dot(edge1, h);
  if (abs(a) < FLT_EPSILON) {
    return res; // The ray is parallel to the triangle
  }
  f = 1.0 / a;
  s = ray.position - triangle.a.xyz;
  u = f * dot(s, h);
  if (u < 0.0 || u > 1.0)
    return res;
  q = cross(s, edge1);
  v = f * dot(ray.direction, q);
  if (v < 0.0 || u + v > 1.0) {
    return res;
  }
  float t = f * dot(edge2, q); // Find out where the intersection point is on the line
  if (t < 0) {
    return res;
  }

  res.hitted = true;
  res.distance = t;
  res.normal = normalize(cross(edge1, edge2));
  res.position = ray.position + ray.direction * t;
  return res;
}

struct RayAABBResult {
  bool hitted;
};

// https://www.bilibili.com/video/BV1X7411F744?p=13 1:09:48
RayAABBResult RayAABB(Ray ray, AABB aabb) {
  RayAABBResult res;
  res.hitted = false;

  float t_enter_x = FLT_LOWEST;
  float t_enter_y = FLT_LOWEST;
  float t_enter_z = FLT_LOWEST;
  float t_exit_x = FLT_MAX;
  float t_exit_y = FLT_MAX;
  float t_exit_z = FLT_MAX;
  if (ray.direction.x > 0) {
    t_enter_x = (aabb.minimum.x - ray.position.x) / ray.direction.x;
    t_exit_x = (aabb.maximum.x - ray.position.x) / ray.direction.x;
  } else if (ray.direction.x < 0) {
    t_enter_x = (aabb.maximum.x - ray.position.x) / ray.direction.x;
    t_exit_x = (aabb.minimum.x - ray.position.x) / ray.direction.x;
  }

  if (ray.direction.y > 0) {
    t_enter_y = (aabb.minimum.y - ray.position.y) / ray.direction.y;
    t_exit_y = (aabb.maximum.y - ray.position.y) / ray.direction.y;
  } else if (ray.direction.y < 0) {
    t_enter_y = (aabb.maximum.y - ray.position.y) / ray.direction.y;
    t_exit_y = (aabb.minimum.y - ray.position.y) / ray.direction.y;
  }

  if (ray.direction.z > 0) {
    t_enter_z = (aabb.minimum.z - ray.position.z) / ray.direction.z;
    t_exit_z = (aabb.maximum.z - ray.position.z) / ray.direction.z;
  } else if (ray.direction.z < 0) {
    t_enter_z = (aabb.maximum.z - ray.position.z) / ray.direction.z;
    t_exit_z = (aabb.minimum.z - ray.position.z) / ray.direction.z;
  }

  float t_enter = max(max(t_enter_x, t_enter_y), t_enter_z);
  float t_exit = min(min(t_exit_x, t_exit_y), t_exit_z);

  res.hitted = (t_enter <= t_exit && t_exit >= 0);

  return res;
}
