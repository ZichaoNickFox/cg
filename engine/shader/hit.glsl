struct HitSphereResult {
  bool hitted;
  vec3 pos_ws;
  float dist;
  vec3 normal_ws;
};

HitSphereResult hit_sphere(SphereGeometry sphere_geometry, vec3 origin_ws, vec3 dir_ws, float limit) {
  HitSphereResult res;
  res.hitted = false;
  vec3 normalized_dir_ws = normalize(dir_ws);
  vec3 oc = sphere_geometry.center_pos_ws - origin_ws;
  float a = dot(normalized_dir_ws, normalized_dir_ws);
  float b = -2 * dot(normalized_dir_ws, oc);
  float c = dot(oc, oc) - sphere_geometry.radius * sphere_geometry.radius;
  float delta = b * b - 4 * a * c;
  if (delta >= 0) {
    float sqrtd = sqrt(delta);
    float root1 = (-b - sqrtd) / (2 * a);
    float root2 = (-b + sqrtd) / (2 * a);
    if (root1 > 0 && root1 <= limit) {
      res.dist = root1;
      res.pos_ws = origin_ws + res.dist * normalized_dir_ws;
      res.hitted = true;
      res.normal_ws = normalize(res.pos_ws - sphere_geometry.center_pos_ws);
    } else if (root2 > 0 && root2 <= limit) {
      res.dist = root2;
      res.pos_ws = origin_ws + res.dist * normalized_dir_ws;
      res.hitted = true;
      res.normal_ws = normalize(res.pos_ws - sphere_geometry.center_pos_ws);
    }
  }
  return res;
}
