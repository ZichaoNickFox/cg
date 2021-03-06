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

  vec3 inverse_direction = 1.0 / ray.direction;
  vec3 m = (aabb.maximum.xyz - ray.position) * inverse_direction;
  vec3 n = (aabb.minimum.xyz - ray.position) * inverse_direction;
  vec3 t_max = max(m, n);
  vec3 t_min = min(m, n);
  float t_enter = max(max(t_min.x, t_min.y), t_min.z);
  float t_exit = min(min(t_max.x, t_max.y), t_max.z);

  res.hitted = (t_enter <= t_exit && t_exit >= 0);

  return res;
}
