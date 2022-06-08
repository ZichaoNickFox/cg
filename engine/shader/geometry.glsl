// Ray
// Sphere
// Plane

// RaySphereResult
// ray_sphere
// RayPlaneResult
// ray_plane
// RayAABBResult
// ray_AABB
// RayTriangle
// ray_triangle

struct Ray {
  vec3 origin;
  vec3 dir; // must normalized
};

struct Sphere {
  int id;
  vec3 center_pos;
  vec4 color;
  float radius;
};

struct Plane {
  vec3 point;
  vec3 normal; // must normalized
};

struct RaySphereResult {
  bool hitted;
  vec3 pos;
  float dist;
  vec3 normal;
};

// https://www.bilibili.com/video/BV1X7411F744?p=13 - 0:31:19
RaySphereResult ray_sphere(Sphere sphere, Ray ray, float limit) {
  RaySphereResult res;
  res.hitted = false;
  vec3 normalized_dir = normalize(ray.dir);
  vec3 oc = sphere.center_pos - ray.origin;
  float a = dot(normalized_dir, normalized_dir);
  float b = -2 * dot(normalized_dir, oc);
  float c = dot(oc, oc) - sphere.radius * sphere.radius;
  float delta = b * b - 4 * a * c;
  if (delta >= 0) {
    float sqrtd = sqrt(delta);
    float root1 = (-b - sqrtd) / (2 * a);
    float root2 = (-b + sqrtd) / (2 * a);
    if (root1 > 0 && root1 <= limit) {
      res.dist = root1;
      res.pos= ray.origin + res.dist * normalized_dir;
      res.hitted = true;
      res.normal = normalize(res.pos- sphere.center_pos);
    } else if (root2 > 0 && root2 <= limit) {
      res.dist = root2;
      res.pos = ray.origin + res.dist * normalized_dir;
      res.hitted = true;
      res.normal = normalize(res.pos - sphere.center_pos);
    }
  }
  return res;
}

struct RayPlaneResult {
  bool hitted;
  vec3 pos;
  float dist;
};

// https://www.bilibili.com/video/BV1X7411F744?p=13 - 1:14:45
RayPlaneResult ray_plane(Ray ray, Plane plane) {
  RayPlaneResult res;
  res.dist = (plane.point.x - ray.origin.x) / ray.dir.x;
  res.hitted = res.dist >= 0.0;
  if (res.hitted) {
    res.pos = ray.origin + ray.dir * res.dist;
  }
  return res;
}
