#include "playground/renderer_scene/shadow_scene.h"

#include <algorithm>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "imgui.h"

#include "renderer/framebuffer_attachment.h"
#include "renderer/shader.h"

using namespace cg;

namespace {
constexpr char kShadowModeShadowMap[] = "ShadowMap";
constexpr char kShadowModePCF[] = "PCF";
constexpr char kShadowModePCSS[] = "PCSS";
const char* kShadowModeNames[] = {kShadowModeShadowMap, kShadowModePCF, kShadowModePCSS};

const char* ShadowModeName(ShadowScene::ShadowMode shadow_mode) {
  return kShadowModeNames[shadow_mode];
}

class ShadowSceneDepthShader : public RenderShader {
 public:
  ShadowSceneDepthShader(const glm::mat4& light_view_project, const Scene& scene, const Object& object)
      : RenderShader(scene, "shadow_scene_depth") {
    ShaderProgramBindings bindings;
    AppendModelBindings(object.transform.GetModelMatrix(), &bindings);
    bindings.SetMat4("light_view_project", light_view_project);
    DrawBindings(bindings, scene, object);
  }
};

class ShadowSceneShader : public RenderShader {
 public:
  struct Param {
    glm::mat4 light_view_project;
    Texture shadow_map;
    glm::vec3 light_direction_ws;
    glm::vec3 light_color;
    float light_intensity = 1.0f;
    float ambient_strength = 0.15f;
    float shadow_bias = 0.001f;
    float normal_bias_scale = 0.01f;
    int shadow_mode = ShadowScene::kShadowMap;
    float pcf_filter_radius = 1.0f;
    float pcss_light_size_uv = 0.0f;
    float pcss_blocker_search_scale = 1.0f;
    float pcss_min_filter_radius = 0.0f;
    float pcss_max_filter_radius = 1.0f;
    bool disable_shadow = false;
    bool debug_show_shadow_factor = false;
  };

  ShadowSceneShader(const Param& param, const Scene& scene, const Object& object)
      : RenderShader(scene, "shadow_scene") {
    ShaderProgramBindings bindings;
    AppendRenderObjectBindings(object, scene.camera(), &bindings);
    AppendMaterialIndexBindings(object.material_index, &bindings);
    bindings.SetMat4("light_view_project", param.light_view_project);
    bindings.SetTexture("shadow_map", param.shadow_map);
    bindings.SetVec3("light_direction_ws", param.light_direction_ws);
    bindings.SetVec3("light_color", param.light_color);
    bindings.SetFloat("light_intensity", param.light_intensity);
    bindings.SetFloat("ambient_strength", param.ambient_strength);
    bindings.SetFloat("shadow_bias", param.shadow_bias);
    bindings.SetFloat("normal_bias_scale", param.normal_bias_scale);
    bindings.SetInt("shadow_mode", param.shadow_mode);
    bindings.SetFloat("pcf_filter_radius", param.pcf_filter_radius);
    bindings.SetFloat("pcss_light_size_uv", param.pcss_light_size_uv);
    bindings.SetFloat("pcss_blocker_search_scale", param.pcss_blocker_search_scale);
    bindings.SetFloat("pcss_min_filter_radius", param.pcss_min_filter_radius);
    bindings.SetFloat("pcss_max_filter_radius", param.pcss_max_filter_radius);
    bindings.SetBool("disable_shadow", param.disable_shadow);
    bindings.SetBool("debug_show_shadow_factor", param.debug_show_shadow_factor);
    DrawBindings(bindings, scene, object);
  }
};

}  // namespace

void ShadowScene::OnEnter() {
  camera_->mutable_transform()->SetTranslation(glm::vec3(0.0f, 3.0f, 8.0f));
  camera_->mutable_transform()->SetRotation(glm::angleAxis(-0.28f, glm::vec3(1.0f, 0.0f, 0.0f)));

  shadow_fbo_.Init({shadow_map_size_, {kAttachmentDepth}});
  main_fbo_.Init({io().framebuffer_size(), {kAttachmentColor, kAttachmentDepth}});

  object_repo_.AddOrReplace(object_metas_);
}

void ShadowScene::OnUpdate() {
  int shadow_mode = shadow_mode_;
  ImGui::Text("Shadow Scene");
  ImGui::Combo("shadow mode", &shadow_mode, kShadowModeNames, IM_ARRAYSIZE(kShadowModeNames));
  shadow_mode_ = static_cast<ShadowMode>(shadow_mode);
  ImGui::Text("Current mode: %s", ShadowModeName(shadow_mode_));
  ImGui::Text("PCF shader: playground/shader/shadow_scene/shadow_scene_pcf.glsl");
  ImGui::Text("PCSS shader: playground/shader/shadow_scene/shadow_scene_pcss.glsl");
  ImGui::Text("Shadow map: %d x %d", shadow_map_size_.x, shadow_map_size_.y);
  ImGui::Separator();

  ImGui::SliderFloat3("light position", &light_position_.x, -10.0f, 10.0f);
  ImGui::SliderFloat3("light target", &light_target_.x, -5.0f, 5.0f);
  ImGui::SliderFloat("light ortho width", &light_ortho_width_, 2.0f, 30.0f);
  ImGui::SliderFloat("light near", &light_near_clip_, 0.01f, 5.0f);
  ImGui::SliderFloat("light far", &light_far_clip_, 2.0f, 60.0f);
  ImGui::Separator();

  ImGui::SliderFloat("light intensity", &light_intensity_, 0.0f, 4.0f);
  ImGui::SliderFloat("ambient strength", &ambient_strength_, 0.0f, 1.0f);
  ImGui::SliderFloat("shadow bias", &shadow_bias_, 0.0f, 0.02f, "%.5f");
  ImGui::SliderFloat("normal bias scale", &normal_bias_scale_, 0.0f, 0.1f, "%.4f");
  if (shadow_mode_ == kPCF) {
    ImGui::SliderFloat("pcf filter radius", &pcf_filter_radius_, 0.0f, 6.0f);
  } else if (shadow_mode_ == kPCSS) {
    ImGui::SliderFloat("pcss light size ws", &pcss_light_size_ws_, 0.0f, 4.0f);
    ImGui::SliderFloat("pcss blocker search scale", &pcss_blocker_search_scale_, 0.0f, 8.0f);
    ImGui::SliderFloat("pcss min filter radius", &pcss_min_filter_radius_, 0.0f, 8.0f);
    ImGui::SliderFloat("pcss max filter radius", &pcss_max_filter_radius_, 0.0f, 16.0f);
  }
  ImGui::Separator();

  ImGui::Checkbox("show shadow map", &debug_show_shadow_map_);
  ImGui::Checkbox("disable shadow", &debug_disable_shadow_);
  ImGui::Checkbox("show shadow factor", &debug_show_shadow_factor_);
}

void ShadowScene::OnRender() {
  RenderShadowMap();
  RenderMainPass();

  if (debug_show_shadow_map_) {
    FullscreenQuadShader({shadow_fbo_.GetTexture("depth")}, *this);
  } else {
    FullscreenQuadShader({main_fbo_.GetTexture("color")}, *this);
  }
}

void ShadowScene::RenderShadowMap() {
  auto shadow_pass = shadow_fbo_.BindScoped();

  const glm::mat4 light_view_project = GetLightViewProject();
  for (const Object& object : object_repo_.GetObjects()) {
    ShadowSceneDepthShader(light_view_project, *this, object);
  }
}

void ShadowScene::RenderMainPass() {
  auto main_pass = main_fbo_.BindScoped();

  float light_size_uv = 0.0f;
  if (light_ortho_width_ > 1e-6f) {
    light_size_uv = pcss_light_size_ws_ / light_ortho_width_;
  }
  float pcss_min_filter_radius = std::min(pcss_min_filter_radius_, pcss_max_filter_radius_);
  float pcss_max_filter_radius = std::max(pcss_min_filter_radius_, pcss_max_filter_radius_);

  ShadowSceneShader::Param param;
  param.light_view_project = GetLightViewProject();
  param.shadow_map = shadow_fbo_.GetTexture("depth");
  param.light_direction_ws = GetLightDirection();
  param.light_color = light_color_;
  param.light_intensity = light_intensity_;
  param.ambient_strength = ambient_strength_;
  param.shadow_bias = shadow_bias_;
  param.normal_bias_scale = normal_bias_scale_;
  param.shadow_mode = shadow_mode_;
  param.pcf_filter_radius = pcf_filter_radius_;
  param.pcss_light_size_uv = light_size_uv;
  param.pcss_blocker_search_scale = pcss_blocker_search_scale_;
  param.pcss_min_filter_radius = pcss_min_filter_radius;
  param.pcss_max_filter_radius = pcss_max_filter_radius;
  param.disable_shadow = debug_disable_shadow_;
  param.debug_show_shadow_factor = debug_show_shadow_factor_;

  for (const Object& object : object_repo_.GetObjects()) {
    ShadowSceneShader(param, *this, object);
  }
  LinesShader({}, *this, CoordinatorMesh());
}

glm::mat4 ShadowScene::GetLightViewProject() const {
  glm::vec3 light_delta = light_target_ - light_position_;
  if (glm::dot(light_delta, light_delta) < 1e-6f) {
    light_delta = glm::vec3(-1.0f, -1.0f, -1.0f);
  }
  glm::vec3 light_direction = glm::normalize(light_delta);
  glm::vec3 light_focus = light_position_ + light_direction;
  glm::vec3 up = (glm::abs(glm::dot(light_direction, glm::vec3(0.0f, 1.0f, 0.0f))) > 0.99f)
                     ? glm::vec3(0.0f, 0.0f, 1.0f)
                     : glm::vec3(0.0f, 1.0f, 0.0f);
  glm::mat4 light_view = glm::lookAtRH(light_position_, light_focus, up);
  float half_width = light_ortho_width_ * 0.5f;
  float far_clip = std::max(light_far_clip_, 0.02f);
  float near_clip = std::max(0.01f, std::min(light_near_clip_, far_clip - 0.01f));
  glm::mat4 light_project =
      glm::ortho(-half_width, half_width, -half_width, half_width, near_clip, far_clip);
  return light_project * light_view;
}

glm::vec3 ShadowScene::GetLightDirection() const {
  glm::vec3 light_delta = light_target_ - light_position_;
  if (glm::dot(light_delta, light_delta) < 1e-6f) {
    light_delta = glm::vec3(-1.0f, -1.0f, -1.0f);
  }
  return glm::normalize(light_delta);
}

void ShadowScene::OnExit() {
}
