#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "renderer/framebuffer.h"
#include "renderer/object.h"
#include "renderer/scene.h"

class ShadowScene : public cg::Scene {
 public:
  enum ShadowMode {
    kShadowMap = 0,
    kPCF = 1,
    kPCSS = 2,
  };

  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  void RenderShadowMap();
  void RenderMainPass();
  glm::mat4 GetLightViewProject() const;
  glm::vec3 GetLightDirection() const;

  std::vector<cg::ObjectMeta> object_metas_ = {
    {"ground", {glm::vec3(0.0f, -1.0f, 0.0f), glm::quat(), glm::vec3(8.0f, 1.0f, 8.0f)}, "plane", "silver"},
    {"receiver_sphere", {glm::vec3(0.0f, 0.15f, 0.0f), glm::quat(), glm::vec3(0.9f)}, "sphere", "gold"},
    {"occluder_cube", {glm::vec3(1.1f, 1.35f, 0.6f), glm::quat(), glm::vec3(0.7f, 2.4f, 0.7f)}, "cube", "ruby"},
    {"side_cube", {glm::vec3(-1.8f, -0.2f, -1.2f), glm::quat(), glm::vec3(0.75f)}, "cube", "jade"},
  };

  cg::Framebuffer shadow_fbo_;
  cg::Framebuffer main_fbo_;

  glm::ivec2 shadow_map_size_ = {2048, 2048};

  glm::vec3 light_position_ = glm::vec3(4.0f, 6.0f, 3.0f);
  glm::vec3 light_target_ = glm::vec3(0.0f, 0.2f, 0.0f);
  glm::vec3 light_color_ = glm::vec3(1.0f, 0.97f, 0.92f);

  float light_intensity_ = 1.25f;
  float ambient_strength_ = 0.18f;
  float shadow_bias_ = 0.0015f;
  float normal_bias_scale_ = 0.02f;
  float pcf_filter_radius_ = 1.5f;
  float pcss_light_size_ws_ = 0.45f;
  float pcss_blocker_search_scale_ = 2.0f;
  float pcss_min_filter_radius_ = 0.75f;
  float pcss_max_filter_radius_ = 6.0f;
  float light_ortho_width_ = 12.0f;
  float light_near_clip_ = 0.1f;
  float light_far_clip_ = 24.0f;

  ShadowMode shadow_mode_ = kShadowMap;
  bool debug_show_shadow_map_ = false;
  bool debug_disable_shadow_ = false;
  bool debug_show_shadow_factor_ = false;
};
