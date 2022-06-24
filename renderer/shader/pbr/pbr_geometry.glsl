float GeometrySchlickGGX(float NdotV, float roughness) {
//  float r = (roughness + 1.0);
//  float k = (r * r) / 8.0;

  float nom = NdotV;
  //float denom = NdotV * (1.0 - k) + k;
  float denom = NdotV * (1.0 - roughness) + roughness;
  return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float k) {
  float NdotV = max(dot(N, V), 0.0);
  float NdotL = max(dot(N, L), 0.0);
  float ggx1 = GeometrySchlickGGX(NdotV, k);
  float ggx2 = GeometrySchlickGGX(NdotL, k);
  return ggx1 * ggx2;
}