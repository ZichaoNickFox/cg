#include "renderer/shader/version.glsl"

#include "renderer/shader/camera.glsl"
#include "renderer/shader/material.glsl"
#include "playground/shader/shadow_scene/shadow_scene_common.glsl"
#include "playground/shader/shadow_scene/shadow_scene_pcf.glsl"
#include "playground/shader/shadow_scene/shadow_scene_pcss.glsl"

uniform Camera camera;
uniform int material_index;
uniform mat4 light_view_project;
uniform sampler2D shadow_map;
uniform vec3 light_direction_ws;
uniform vec3 light_color;
uniform float light_intensity;
uniform float ambient_strength;
uniform float shadow_bias;
uniform float normal_bias_scale;
uniform int shadow_mode;
uniform float pcf_filter_radius;
uniform float pcss_light_size_uv;
uniform float pcss_blocker_search_scale;
uniform float pcss_min_filter_radius;
uniform float pcss_max_filter_radius;
uniform bool disable_shadow;
uniform bool debug_show_shadow_factor;

in vec2 texcoord_;
in vec3 position_ws_;
in vec3 normal_ws_;
in vec4 position_light_cs_;
in mat3 TBN_ws_;

out vec4 FragColor;

float EvalShadowByMode(sampler2D shadow_map, vec3 shadow_coord, float bias) {
  if (shadow_mode == SHADOW_MODE_SHADOWMAP) {
    return EvalShadowHard(shadow_map, shadow_coord, bias);
  }
  if (shadow_mode == SHADOW_MODE_PCF) {
    return EvalShadowPcf(shadow_map, shadow_coord, bias, pcf_filter_radius);
  }
  if (shadow_mode == SHADOW_MODE_PCSS) {
    return EvalShadowPcss(shadow_map, shadow_coord, bias, pcss_light_size_uv, pcss_blocker_search_scale,
                          pcss_min_filter_radius, pcss_max_filter_radius);
  }
  return EvalShadowHard(shadow_map, shadow_coord, bias);
}

void main()
{
  Material material = material_repo[material_index];
  vec3 N = MaterialNormal(material, TBN_ws_, normal_ws_, texcoord_);
  vec3 L = normalize(-light_direction_ws);
  vec3 V = normalize(camera.pos_ws - position_ws_);
  vec3 H = normalize(L + V);

  vec4 diffuse = MaterialDiffuse(material, texcoord_);
  vec4 specular = MaterialSpecular(material, texcoord_);
  float shininess = max(MaterialShininess(material, texcoord_), 1.0);

  float ndl = max(dot(N, L), 0.0);
  float specular_term = pow(max(dot(N, H), 0.0), shininess);

  vec3 receiver_position_ws = ApplyNormalBias(position_ws_, N, light_direction_ws, normal_bias_scale);
  vec3 shadow_coord = ShadowClipToUvDepth(light_view_project * vec4(receiver_position_ws, 1.0));
  float visibility = disable_shadow ? 1.0 : EvalShadowByMode(shadow_map, shadow_coord, shadow_bias);

  vec3 ambient = diffuse.rgb * ambient_strength;
  vec3 direct = (diffuse.rgb * ndl + specular.rgb * specular_term) * light_color * light_intensity;
  vec3 color = ambient + direct * visibility;

  if (debug_show_shadow_factor) {
    color = vec3(visibility);
  }

  FragColor = vec4(color, diffuse.a);
}
