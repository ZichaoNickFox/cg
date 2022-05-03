float D_GGX_TR(vec3 N, vec3 H, float roughness) {
  float a = roughness * roughness;
  float a2 = a * a;
  float NdotH = max(dot(N, H), 0.0);
  float NdotH2 = NdotH * NdotH;

  float nom = a2;
  float denom = (NdotH * (a2 - 1.0) + 1.0);
  denom = PI * denom * denom;

  return nom / denom;
}