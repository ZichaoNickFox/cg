struct ShadowModelInput {
  vec4 frag_shadow_pos_light_space;
  float depth_in_texture;
};

float ShadowModel(ShadowModelInput param) {
  vec4 frag_shadow_pos_ndc = param.frag_shadow_pos_light_space / param.frag_shadow_pos_light_space.w;
  vec4 frag_shadow_pos_screen_space = frag_shadow_pos_ndc * 0.5 + 0.5;

  bool in_shadow = param.depth_in_texture + 0.001 < frag_shadow_pos_screen_space.z;
  if (in_shadow) {
    return 0.5;
  } else {
    return 1.0;
  }
}

vec2 GetShadowMapTexcoord(vec4 frag_shadow_pos_light_space) {
  return ((frag_shadow_pos_light_space / frag_shadow_pos_light_space.w) * 0.5 + 0.5).xy;
}