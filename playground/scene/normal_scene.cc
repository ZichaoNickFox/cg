#include "playground/scene/normal_scene.h"

#include <glm/glm.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <math.h>
#include <memory>

#include "engine/transform.h"
#include "playground/scene/common.h"

void NormalScene::OnEnter(Context *context)
{
  const glm::vec3 kLightColor = glm::vec3(1.0, 1.0, 1.0);
  const glm::vec3 kLightPos = glm::vec3(2, 2, 0);
  const glm::vec3 kLightScale = glm::vec3(0.4, 0.4, 0.4);
  point_light_.mutable_transform()->SetTranslation(kLightPos);
  point_light_.mutable_transform()->SetScale(kLightScale);
  point_light_.SetColor(kLightColor);

  plane_.mutable_transform()->SetScale(glm::vec3(3, 3, 3));
  // plane_.mutable_transform()->SetRotation(glm::angleAxis(float(M_PI) / 2, glm::vec3(1.0, 0.0, -1.0)));

  sphere_.mutable_transform()->SetTranslation(glm::vec3(3, 1, 1));

  camera_->mutable_transform()->SetTranslation(glm::vec3(-4.8, 6.1, 5.8));
  camera_->mutable_transform()->SetRotation(glm::quat(0.88, -0.30, -0.32, -0.11));
  context->PushCamera(camera_);

  glEnable(GL_DEPTH_TEST);
}

void NormalScene::OnUpdate(Context *context)
{
  OnUpdateCommon _(context, "NormalScene");

  ImGui::ColorEdit3("light_color", (float*)&light_color_);
  if (ImGui::Button("gold")) {
    material_property_name_ = "gold";
  }
  ImGui::SameLine();
  if (ImGui::Button("silver")) {
    material_property_name_ = "silver";
  }
  ImGui::SameLine();
  if (ImGui::Button("jade")) {
    material_property_name_ = "jade";
  }
  ImGui::SameLine();
  if (ImGui::Button("rube")) {
    material_property_name_ = "ruby";
  }
  ImGui::Checkbox("blinn_phong", &use_blinn_phong_);

  point_light_.SetColor(light_color_);
  point_light_.OnUpdate(context);

  coord_.OnUpdate(context);
}

void NormalScene::OnRender(Context *context)
{
  coord_.OnRender(context);
  point_light_.OnRender(context);

  PhongShader::Param phong;
  phong.light_info = ShaderLightInfo(point_light_);
  phong.use_blinn_phong = use_blinn_phong_;
  phong.texture_normal = context->GetTexture("brickwall_normal");
  phong.texture_diffuse = context->GetTexture("brickwall");

  PhongShader(phong, context, &plane_);
  plane_.OnRender(context);
  NormalShader({0.1}, context, &plane_);
  plane_.OnRender(context);

  PhongShader(phong, context, &sphere_);
  sphere_.OnRender(context);
  NormalShader({0.1}, context, &sphere_);
  sphere_.OnRender(context);
}

void NormalScene::OnExit(Context *context)
{
  point_light_.OnDestory(context);
  plane_.OnDestory(context);
  sphere_.OnDestory(context);
  coord_.OnDestory(context);
  context->PopCamera();
}