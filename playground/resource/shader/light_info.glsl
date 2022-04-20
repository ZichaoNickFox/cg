struct Light {
  vec3 color;
  vec3 pos;
  float constant;   // attenuation
  float linear;     // attenuation
  float quadratic;  // attenuation
};