#include "engine/shader/version.glsl"
#include "engine/shader/random.glsl"
#include "engine/shader/sample.glsl"

out vec4 FragColor;
in vec2 texcoord_;

vec2 integrateBRDF(float NdotV, float roughness) {
  vec3 V;
  V.x = sqrt(1.0 - NdotV * NdotV);
  V.y = 0.0;
  V.z = NdotV;

  float A = 0.0;
  float B = 0.0;

  vec3 N = vec3(0, 0, 1);

  const int sample_count = 1024;
  for (int i = 0; i < sample_count; ++i) {
    vec2 Xi = Hammersley(i, sample_count);
    vec3 H = ImportancleSampleGGX(Xi, N, roughness);
    vec3 L = normalize(2.0 * dot(V, H) * H - V);

    float NdotL = max(L.z, 0.0);
    float NdotH = max(H.z, 0.0);
    float VdotH = max(dot(V, H), 0.0);

    if (NdotL > 0) {
      float G = GeometrySmith(N, V, L, roughness);
      float G_Vis = (G * VdotH) / (NdotH * NdotV);
      float Fc = pow(1.0 - VdotH, 5.0);

      A += (1.0 - Fc) * G_Vis;
      B += Fc * G_Vis;
    }
  }
  A /= float(sample_count);
  B /= float(sample_count);
  return vec2(A, B);
}

void main() {
  vec2 integrateBRDF = integrateBRDF(texcoord_.x, texcoord_.y);
  FragColor = vec4(integrateBRDF, 0.0, 1.0);
}