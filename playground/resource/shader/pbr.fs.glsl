#version 330 core

out vec4 FragColor;

in vec3 world_pos_;
in vec3 normal_;

uniform vec3 camera_pos;
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;

const float PI = 3.1415926;

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

float GeometrySchlickGGX(float NdotV, float roughness) {
  float r = (roughness + 1.0);
  float k = (r * r) / 8.0;

  float nom = NdotV;
  float denom = NdotV * (1.0 - k) + k;
  return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float k) {
  float NdotV = max(dot(N, V), 0.0);
  float NdotL = max(dot(N, L), 0.0);
  float ggx1 = GeometrySchlickGGX(NdotV, k);
  float ggx2 = GeometrySchlickGGX(NdotL, k);
  return ggx1 * ggx2;
}

vec3 FresnelSchlink(float cosTheta, vec3 F0) {
  return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
  vec3 N = normalize(normal_);
  vec3 V = normalize(camera_pos - world_pos_);

  vec3 Lo = vec3(0.0);
  for (int i = 0; i < 4; ++i) {
    vec3 L = normalize(light_position[i] - world_pos_);
    vec3 H = normalize(V + L);
    float distance = length(light_position[i] - world_pos_);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = light_color[i] * attenuation;

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    vec3 F = FresnelSchlink(max(dot(H, V), 0.0), F0);

    float NDF = D_GGX_TR(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);

    vec3 nominator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    vec3 specular = nominator / denominator;

    vec3 kS = F:
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;
  }
  vec3 ambient = vec3(0.03) * albedo * ao;
  vec3 color = ambient + Lo;
  color = color / (color + vec3(1.0));
  color = pow(color, vec3(1.0 / 2.2));

  FragColor = vec4(color, 1);
}