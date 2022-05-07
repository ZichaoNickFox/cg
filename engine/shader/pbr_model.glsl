uniform samplerCube texture_irradiance_cubemap;
uniform samplerCube texture_prefiltered_color_cubemap;
uniform sampler2D texture_BRDF_integration_map;
uniform int light_count;
uniform Light lights[200];

const float PI = 3.1415926;

struct PbrModelInput {
  vec3 view_pos;
  vec3 frag_world_pos;

  vec3 albedo;
  float roughness;
  float metallic;
  vec3 ao;
  vec3 normal;
};

vec3 PbrModel(PbrModelInput param) {
  vec3 N = normalize(param.normal);
  vec3 V = normalize(param.view_pos - param.frag_world_pos);

  vec3 F0 = vec3(0.04);
  F0 = mix(F0, param.albedo, param.metallic);

  vec3 Lo = vec3(0.0);
  for (int i = 0; i < light_count; ++i) {
    Light light = lights[i];

    vec3 L = normalize(light.pos - param.frag_world_pos);
    vec3 H = normalize(V + L);
    float distance = length(light.pos - param.frag_world_pos);
    float attenuation = 1.0 / (distance * distance) * light.quadratic +
        distance * light.linear + distance * light.constant;
    vec3 radiance = light.color * attenuation;

    vec3 F = FresnelSchlink(max(dot(H, V), 0.0), F0);

    float NDF = D_GGX_TR(N, H, param.roughness);
    float G = GeometrySmith(N, V, L, param.roughness);

    vec3 nominator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    vec3 specular = nominator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - param.metallic;

    float NdotL = max(dot(N, L), 0.0);
    Lo += (kD * param.albedo / PI + specular) * radiance * NdotL;
  }
  vec3 R = normalize(reflect(-V, N));
  const float MAX_REFLECTED_LOD = 4.0;
  vec3 prefiltered_color = textureLod(texture_prefiltered_color_cubemap, R, param.roughness * MAX_REFLECTED_LOD).rgb;
  vec3 ambient_F = FresnelSchlinkRoughness(max(dot(N, V), 0.0), F0, param.roughness);

  vec3 ambient_KS = ambient_F;
  vec3 ambient_KD = vec3(1.0) - ambient_KS;

  vec2 BRDF_integrate = texture(texture_BRDF_integration_map, vec2(max(dot(N, V), 0.0), param.roughness)).rg;
  vec3 ambient_specular = prefiltered_color * (ambient_F * BRDF_integrate.x + BRDF_integrate.y);
  vec3 irradiance = texture(texture_irradiance_cubemap, N).rgb;
  vec3 ambient = (ambient_KD * irradiance * param.albedo + ambient_specular) * param.ao;
  vec3 color = ambient + Lo;
  return prefiltered_color;
}