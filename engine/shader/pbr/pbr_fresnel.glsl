vec3 FresnelSchlink(float cosTheta, vec3 F0) {
  return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float FresnelSchlink(float cosTheta, float F0) {
  return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float FresnelSchlink(vec3 V, vec3 N, float F0) {
  float cosTheta = max(dot(V, N), 0.0);
  return FresnelSchlink(cosTheta, F0);
}

vec3 FresnelSchlinkRoughness(float cosTheta, vec3 F0, float roughness) {
  return F0 + (max(vec3(1.0 - roughness), F0)- F0) * pow(1.0 - cosTheta, 5.0);
}