#include "playground/scene/pbr_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "engine/transform.h"
#include "playground/scene/common.h"

void PbrScene::OnEnter(Context *context)
{
  const glm::vec4 kLightColor = glm::vec4(0.0, 1.0, 0.0, 1.0);
  const glm::vec3 kLightScale = glm::vec3(0.4, 0.4, 0.4);

  std::vector<glm::vec3> light_poses{glm::vec3(2, 2, 0), glm::vec3(2, 0, 2),
                                     glm::vec3(0, 2, 2), glm::vec3(2, 2, 2)};
  for (int i = 0; i < light_num; ++i) {
    point_lights_.push_back(PointLight());
    point_lights_[i].mutable_transform()->SetTranslation(light_poses[i]);
    point_lights_[i].mutable_transform()->SetScale(kLightScale);
    point_lights_[i].SetColor(kLightColor);
  }

  camera_->mutable_transform()->SetTranslation(glm::vec3(2.97, 3.95, 6.76));
  camera_->mutable_transform()->SetRotation(glm::quat(0.95, -0.21, 0.18, 0.04));
  camera_->SetFarClip(200);
  context->SetCamera(camera_);

  skybox_.mutable_transform()->SetScale(glm::vec3(100, 100, 100));

  plane_.mutable_transform()->SetScale(glm::vec3(5.0, 5.0, 5.0));

  glEnable(GL_DEPTH_TEST);
}

void PbrScene::OnUpdate(Context *context)
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

  ImGui::SliderFloat("metallic", &metallic, 0.0, 1.0);
  ImGui::SliderFloat("roughness", &roughness, 0.0, 1.0);
  ImGui::SliderFloat("ao", &ao, 0.0, 1.0);

  sphere_.mutable_transform()->SetTranslation(glm::vec3(0, 1, 0));
}

void PbrScene::OnRender(Context *context)
{
  for (int i = 0; i < light_num; ++i) {
    ColorShader({}, context, &point_lights_[i]);
    point_lights_[i].OnRender(context);
  }

  PbrShader::Param pbr;
  pbr.albedo = albedo_;
  pbr.metallic = metallic;
  pbr.roughness = roughness;
  pbr.light_info = ShaderLightInfo(point_lights_);
  pbr.texture_irradiance_cubemap = context->GetTexture("pbr_irradiance_cubemap");
  pbr.texture_prefiltered_color_cubemap = context->GetTexture("pbr_prefiltered_color_cubemap");
  pbr.texture_BRDF_integration_map = context->GetTexture("pbr_BRDF_integration_map");

  PbrShader(pbr, context, &sphere_);
  sphere_.OnRender(context);

  coord_.OnRender(context);
  CubemapShader({context->GetTexture("pbr_environment_cubemap")}, context, &skybox_);
  skybox_.OnRender(context);

  PbrShader(pbr, context, &plane_);
  plane_.OnRender(context);
}

void PbrScene::OnExit(Context *context)
{
  for (int i = 0; i < light_num; ++i) {
    point_lights_[i].OnDestory(context);
  }
  coord_.OnDestory(context);
  skybox_.OnDestory(context);
  context->SetCamera(nullptr);
  plane_.OnDestory(context);
}