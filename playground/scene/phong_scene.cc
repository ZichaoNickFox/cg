#include "playground/scene/phong_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "engine/transform.h"
#include "playground/scene/common.h"

void PhongScene::OnEnter(Context *context)
{
  const glm::vec3 kLightColor = glm::vec3(1.0, 1.0, 1.0);
  const glm::vec3 kLightPos = glm::vec3(2, 2, 0);
  const glm::vec3 kLightScale = glm::vec3(0.4, 0.4, 0.4);
  point_light_.mutable_transform()->SetTranslation(kLightPos);
  point_light_.mutable_transform()->SetScale(kLightScale);
  point_light_.SetColor(kLightColor);

  material_light_info_.light_poses.push_back(kLightPos);
  material_light_info_.light_colors.push_back(kLightColor);
  material_light_info_.light_attenuation_metres.push_back(100);

  const glm::vec3 kCubePosition = glm::vec3(0, 0.5, 0);
  cube_.mutable_transform()->SetTranslation(kCubePosition);
  plane_.mutable_transform()->SetScale(glm::vec3(3, 3, 3));

  camera_->mutable_transform()->SetTranslation(glm::vec3(-4.8, 6.1, 5.8));
  camera_->mutable_transform()->SetRotation(glm::quat(0.88, -0.30, -0.32, -0.11));
  context->PushCamera(camera_);

  glEnable(GL_DEPTH_TEST);
}

void PhongScene::OnUpdate(Context *context)
{
  OnUpdateCommon _(context, "PhongScene");

  ImGui::ColorEdit3("light_color", (float*)&light_color_);
  ImGui::Separator();
  ImGui::Text("camera_location %s", glm::to_string(context->camera().transform().translation()).c_str());
  ImGui::Text("camera_rotation %s", glm::to_string(context->camera().transform().rotation()).c_str());
  ImGui::Text("camera_front %s", glm::to_string(context->camera().front()).c_str());
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
  ImGui::Checkbox("blinn_phong", &blinn_phong_);

  material_light_info_.light_colors.assign(material_light_info_.light_colors.size(), light_color_);

  PhongMaterialPrefab prefab;
  prefab.material_propery_name = material_property_name_;
  prefab.material_light_info = material_light_info_;
  prefab.material_light_info.use_blinn_phong = blinn_phong_;

  material_prefab::UpdatePhongMaterial(context, prefab, cube_.mutable_material());
  material_prefab::UpdatePhongMaterial(context, prefab, plane_.mutable_material());

  cube_.OnUpdate(context);
  point_light_.SetColor(light_color_);
  point_light_.OnUpdate(context);
}

void PhongScene::OnRender(Context *context)
{
  cube_.OnRender(context);
  point_light_.OnRender(context);
  plane_.OnRender(context);
}

void PhongScene::OnExit(Context *context)
{
  cube_.OnDestory(context);
  point_light_.OnDestory(context);
  plane_.OnDestory(context);
  context->PopCamera();
}