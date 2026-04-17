bool FindAverageBlockerDepth(
    sampler2D shadow_map,
    vec3 shadow_coord,
    float bias,
    float search_radius_uv,
    out float average_blocker_depth) {
  float blocker_depth_sum = 0.0;
  int blocker_count = 0;

  for (int i = 0; i < SHADOW_POISSON_SAMPLE_COUNT; ++i) {
    vec2 sample_uv = shadow_coord.xy + SHADOW_POISSON_DISK[i] * search_radius_uv;
    if (!ShadowUvInRange(sample_uv)) {
      continue;
    }

    float sample_depth = texture(shadow_map, sample_uv).r;
    if (sample_depth < shadow_coord.z - bias) {
      blocker_depth_sum += sample_depth;
      blocker_count += 1;
    }
  }

  if (blocker_count == 0) {
    average_blocker_depth = 0.0;
    return false;
  }

  average_blocker_depth = blocker_depth_sum / float(blocker_count);
  return true;
}

float EvalShadowPcss(
    sampler2D shadow_map,
    vec3 shadow_coord,
    float bias,
    float light_size_uv,
    float blocker_search_scale,
    float min_filter_radius,
    float max_filter_radius) {
  if (!ShadowUvInRange(shadow_coord.xy)) {
    return 1.0;
  }
  if (shadow_coord.z <= 0.0 || shadow_coord.z >= 1.0) {
    return 1.0;
  }

  float texel_size = ShadowTexelSizeScalar(shadow_map);
  float search_radius_uv = max(light_size_uv * blocker_search_scale * max(shadow_coord.z, 0.1), texel_size);

  float average_blocker_depth = 0.0;
  if (!FindAverageBlockerDepth(shadow_map, shadow_coord, bias, search_radius_uv, average_blocker_depth)) {
    return 1.0;
  }

  float receiver_depth = shadow_coord.z;
  float penumbra_ratio = max(receiver_depth - average_blocker_depth, 0.0) / max(average_blocker_depth, 1e-4);
  float filter_radius_uv = max(light_size_uv * penumbra_ratio, texel_size);
  float filter_radius_texels = filter_radius_uv / texel_size;
  filter_radius_texels = clamp(filter_radius_texels, min_filter_radius, max_filter_radius);

  return EvalShadowPcf(shadow_map, shadow_coord, bias, filter_radius_texels);
}
