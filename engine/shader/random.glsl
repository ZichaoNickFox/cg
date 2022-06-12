uniform float time_for_seed;

// https://www.shadertoy.com/view/4djSRW
float hash12(vec2 p)
{
  vec3 p3  = fract(vec3(p.xyx) * .1031);
  p3 += dot(p3, p3.yzx + 33.33);
  return fract((p3.x + p3.y) * p3.z);
}

int i  =0;
float random(){
return 0;
}
