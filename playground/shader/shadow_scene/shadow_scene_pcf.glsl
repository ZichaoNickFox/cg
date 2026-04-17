float EvalShadowPcf(sampler2D shadow_map, vec3 shadow_coord, float bias, float filter_radius) {
  if (!ShadowUvInRange(shadow_coord.xy)) {
    return 1.0;
  }
  if (shadow_coord.z <= 0.0 || shadow_coord.z >= 1.0) {
    return 1.0;
  }

  float filter_radius_texels = max(filter_radius, 0.0);
  if (filter_radius_texels <= 1e-4) {
    return SampleShadowMapCompare(shadow_map, shadow_coord, bias);
  }

  vec2 texel_size = ShadowMapTexelSize(shadow_map);
  vec2 filter_radius_uv = texel_size * filter_radius_texels;

  float visibility = 0.0;
  for (int i = 0; i < SHADOW_POISSON_SAMPLE_COUNT; ++i) {
    vec2 sample_uv = shadow_coord.xy + SHADOW_POISSON_DISK[i] * filter_radius_uv;
    visibility += ShadowCompareAtUv(shadow_map, sample_uv, shadow_coord.z, bias);
  }
  return visibility / float(SHADOW_POISSON_SAMPLE_COUNT);
}
