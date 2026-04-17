const int SHADOW_MODE_SHADOWMAP = 0;
const int SHADOW_MODE_PCF = 1;
const int SHADOW_MODE_PCSS = 2;
const int SHADOW_POISSON_SAMPLE_COUNT = 16;

const vec2 SHADOW_POISSON_DISK[SHADOW_POISSON_SAMPLE_COUNT] = vec2[SHADOW_POISSON_SAMPLE_COUNT](
    vec2(-0.94201624, -0.39906216),
    vec2(0.94558609, -0.76890725),
    vec2(-0.09418410, -0.92938870),
    vec2(0.34495938, 0.29387760),
    vec2(-0.91588581, 0.45771432),
    vec2(-0.81544232, -0.87912464),
    vec2(-0.38277543, 0.27676845),
    vec2(0.97484398, 0.75648379),
    vec2(0.44323325, -0.97511554),
    vec2(0.53742981, -0.47373420),
    vec2(-0.26496911, -0.41893023),
    vec2(0.79197514, 0.19090188),
    vec2(-0.24188840, 0.99706507),
    vec2(-0.81409955, 0.91437590),
    vec2(0.19984126, 0.78641367),
    vec2(0.14383161, -0.14100790));

vec3 ShadowClipToUvDepth(vec4 position_light_cs) {
  vec3 shadow_ndc = position_light_cs.xyz / position_light_cs.w;
  return shadow_ndc * 0.5 + 0.5;
}

bool ShadowUvInRange(vec2 uv) {
  return uv.x >= 0.0 && uv.x <= 1.0 && uv.y >= 0.0 && uv.y <= 1.0;
}

vec2 ShadowMapTexelSize(sampler2D shadow_map) {
  return 1.0 / vec2(textureSize(shadow_map, 0));
}

float ShadowTexelSizeScalar(sampler2D shadow_map) {
  vec2 texel_size = ShadowMapTexelSize(shadow_map);
  return max(texel_size.x, texel_size.y);
}

float ShadowCompareAtUv(sampler2D shadow_map, vec2 uv, float receiver_depth, float bias) {
  if (!ShadowUvInRange(uv)) {
    return 1.0;
  }
  if (receiver_depth <= 0.0 || receiver_depth >= 1.0) {
    return 1.0;
  }

  float closest_depth = texture(shadow_map, uv).r;
  return (receiver_depth - bias) <= closest_depth ? 1.0 : 0.0;
}

float SampleShadowMapCompare(sampler2D shadow_map, vec3 shadow_coord, float bias) {
  return ShadowCompareAtUv(shadow_map, shadow_coord.xy, shadow_coord.z, bias);
}

float EvalShadowHard(sampler2D shadow_map, vec3 shadow_coord, float bias) {
  return SampleShadowMapCompare(shadow_map, shadow_coord, bias);
}

vec3 ApplyNormalBias(vec3 position_ws, vec3 normal_ws, vec3 light_direction_ws, float normal_bias_scale) {
  vec3 N = normalize(normal_ws);
  vec3 L = normalize(-light_direction_ws);
  float slope_scale = 1.0 - max(dot(N, L), 0.0);
  return position_ws + N * slope_scale * normal_bias_scale;
}
