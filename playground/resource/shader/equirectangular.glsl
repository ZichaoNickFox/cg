// local_pos must sphere
// Using cube inscribed sphere is OK. It means each normalize texcoord
vec2 GetEquirectangularTexcoord(vec3 local_pos) {
  local_pos = normalize(local_pos);
  const float pi = 3.1415926;
  const float u_scale = 1 / (pi * 2);
  const float v_scale = 0.5 / (pi / 2);
  const vec2 scale = vec2(u_scale, v_scale);
  vec2 uv = vec2(atan(local_pos.z, local_pos.x), asin(local_pos.y));
  uv *= scale;
  uv += 0.5;
  return uv;
}