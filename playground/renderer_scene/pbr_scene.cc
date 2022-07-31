#include "playground/renderer_scene/pbr_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "renderer/transform.h"
#include "playground/renderer_scene/common.h"

void PbrScene::OnEnter(Scene *context)
{
  const glm::vec4 kLightColor = glm::vec4(0.0, 1.0, 0.0, 1.0);
  const glm::vec3 kLightScale = glm::vec3(0.4, 0.4, 0.4);

  std::vector<glm::vec3> light_poses{glm::vec3(2, 2, 0), glm::vec3(2, 0, 2),
                                     glm::vec3(0, 2, 2), glm::vec3(2, 2, 2)};
  for (int i = 0; i < light_num; ++i) {
    point_lights_.push_back(PointLightObject());
    point_lights_[i].mutable_transform()->SetTranslation(light_poses[i]);
    point_lights_[i].mutable_transform()->SetScale(kLightScale);
    point_lights_[i].SetColor(kLightColor);
  }

  camera_->SetFarClip(200);
  context->SetCamera(camera_.get());

  skybox_.mutable_transform()->SetScale(glm::vec3(100, 100, 100));

  plane_.mutable_transform()->SetScale(glm::vec3(5.0, 5.0, 5.0));

  cube_.mutable_transform()->SetTranslation(glm::vec3(2, 2, 2));

  teapot_.Init(context, "teapot", "teapot");
  for (int i = 0; i < teapot_.model_part_num(); ++i) {
    teapot_.mutable_model_part(i)->mutable_transform()->SetScale(glm::vec3(0.2, 0.2, 0.2));
  }

  glEnable_(GL_DEPTH_TEST);
}

void PbrScene::OnUpdate(Scene *context)
{
  OnUpdateCommon _(context, "PbrScene");

  ImGui::ColorEdit3("light_color", (float*)&light_color_);
  ImGui::ColorEdit3("albedo", (float*)&albedo_);
  
  for (int i = 0; i < light_num; ++i) {
    point_lights_[i].OnUpdate(context);
    point_lights_[i].SetColor(light_color_);
  }

  sphere_.OnUpdate(context);
  coord_.OnUpdate(context);
  skybox_.OnUpdate(context);
  plane_.OnUpdate(context);
  cube_.OnUpdate(context);

  ImGui::SliderFloat("metallic", &metallic_, 0.0, 1.0);
  ImGui::SliderFloat("roughness", &roughness_, 0.0, 1.0);
  ImGui::SliderFloat("ao", &ao_, 0.0, 1.0);

  sphere_.mutable_transform()->SetTranslation(glm::vec3(0, 3, 0));
}

void PbrScene::OnRender(Scene *context)
{
  for (int i = 0; i < light_num; ++i) {
    ColorShader({}, context, &point_lights_[i]);
    point_lights_[i].OnRender(context);
  }

  static PbrShader::Param pbr_cerberus;
  pbr_cerberus.scene_light_info = AsSceneLightInfo(point_lights_);
  pbr_cerberus.texture_irradiance_cubemap = context->GetTexture("pbr_irradiance_tropical");
  pbr_cerberus.texture_prefiltered_color_cubemap = context->GetTexture("pbr_prefiltered_color_tropical");
  pbr_cerberus.texture_BRDF_integration_map = context->GetTexture("pbr_BRDF_integration_map");
  for (int part_index = 0; part_index < teapot_.model_part_num(); ++part_index) {
    PbrShader(&pbr_cerberus, context, teapot_.mutable_model_part(part_index));
    teapot_.mutable_model_part(part_index)->OnRender(context);

    static NormalShader::Param normal_shadow_teapot;
    NormalShader(&normal_shadow_teapot, context, teapot_.mutable_model_part(part_index));
    teapot_.mutable_model_part(part_index)->OnRender(context);
  }

  PbrShader(&pbr_cerberus, context, &cube_);
  cube_.OnRender(context);

  LinesShader({}, context, &coord_);
  coord_.OnRender(context);

  CubemapShader({context->GetTexture("pbr_environment_tropical")}, context, &skybox_);
  skybox_.OnRender(context);
}

void PbrScene::OnExit(Scene *context)
{
  for (int i = 0; i < light_num; ++i) {
    point_lights_[i].OnDestory(context);
  }
  coord_.OnDestory(context);
  skybox_.OnDestory(context);
  context->SetCamera(nullptr);
  plane_.OnDestory(context);
}