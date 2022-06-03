struct HitSphereResult {
  bool hitted;
  vec3 hit_pos_ws;
  float dist;
};

HitSphereResult hit_sphere(SphereGeometry sphere_geometry, vec3 origin_ws, vec3 dir_ws) {
  HitSphereResult res;
  res.hitted = false;
  vec3 normalized_dir_ws = normalize(dir_ws);
  vec3 oc = sphere_geometry.center_pos_ws - origin_ws;
  float a = dot(normalized_dir_ws, normalized_dir_ws);
  float b = 2 * dot(normalized_dir_ws, oc);
  float c = dot(oc, oc) - sphere_geometry.radius * sphere_geometry.radius;
  float delta = b * b - 4 * a * c;
  if (delta >= 0) {
    float sqrtd = sqrt(delta);
    float root1 = (-b - sqrtd) / (2 * a);
    float root2 = (-b + sqrtd) / (2 * a);
    if (root1 > 0) {
      res.dist = root1;
      res.hit_pos_ws = res.dist * normalized_dir_ws + origin_ws;
      res.hitted = true;
    } else if (root2 > 0) {
      res.dist = root2;
      res.hit_pos_ws = res.dist * normalized_dir_ws + origin_ws;
      res.hitted = true;
    }
  }
  return res;
}
