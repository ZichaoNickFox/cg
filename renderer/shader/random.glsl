//internal RNG state 
uvec4 seed;
vec2 pixel;

void InitRNG(vec2 p, int frame_num) {
  pixel = p;
  seed = uvec4(p, uint(frame_num), uint(p.x) + uint(p.y));
}

void pcg4d(inout uvec4 v) {
  v = v * 1664525u + 1013904223u;
  v.x += v.y * v.w; v.y += v.z * v.x; v.z += v.x * v.y; v.w += v.y * v.z;
  v = v ^ (v >> 16u);
  v.x += v.y * v.w; v.y += v.z * v.x; v.z += v.x * v.y; v.w += v.y * v.z;
}

// float hash12(vec2 p)
// {
//  vec3 p3  = fract(vec3(p.xyx) * .1031);
//  p3 += dot(p3, p3.yzx + 33.33);
//  return fract((p3.x + p3.y) * p3.z);
// }

float Random() {
  pcg4d(seed); return float(seed.x) / float(0xffffffffu);
//  return hash12(pixel + frame_num);
}
