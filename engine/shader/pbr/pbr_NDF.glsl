// https://www.jianshu.com/p/d70ee9d4180e/
float D_GGX_TR(vec3 N, vec3 H, float roughness) {
  float a2 = roughness * roughness; 
  float NdotH = max(dot(N, H), 0.0);
  float NdotH2 = NdotH * NdotH;

  float nom = a2;
  float denom = (NdotH2 * (a2 - 1.0) + 1.0);
  const float pi = 3.1415926;
  denom = pi * denom * denom;

  return nom / denom;
}

float D_GGX_TR(vec3 N, vec3 wi, vec3 wo, float roughness) {
  return D_GGX_TR(N, normalize(wi + wo), roughness);
}
