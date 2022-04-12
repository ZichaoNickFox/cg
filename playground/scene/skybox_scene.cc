#include "playground/scene/skybox_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "engine/transform.h"
#include "playground/scene/common.h"

void SkyboxScene::OnEnter(Context *context)
{
  const glm::vec3 kLightColor = glm::vec3(0.0, 1.0, 0.0);
  const glm::vec3 kLightPos = glm::vec3(1, 1, 0);
  const glm::vec3 kLightScale = glm::vec3(0.4, 0.4, 0.4);
  engine::Transform light_transform(kLightPos, glm::quat(glm::vec3(0, 0, 0)), kLightScale);
  light_.SetTransform(light_transform);
  light_.mutable_material()->PushShader(context->GetShader("point_light"));
  light_.mutable_material()->SetVec3("light_color", kLightColor);

  // http://www.barradeau.com/nicoptere/dump/materials.html
  cube_positions_.push_back(glm::vec3(0, 0, 0));
  cube_positions_.push_back(glm::vec3(1, 1, 1));
  cube_positions_.push_back(glm::vec3(2, 2, 1));
  cube_positions_.push_back(glm::vec3(1, 2, 2));
  for (int i = 0; i < cube_positions_.size(); ++i) {
    cubes_.push_back(Cube());
    Cube* cube = &cubes_[i];
    cube->mutable_transform()->SetTranslation(cube_positions_[i]);
    cube->mutable_material()->PushShader(context->GetShader("phong"));
    cube->mutable_material()->SetVec3("lights[0].color", kLightColor);
    cube->mutable_material()->SetVec3("lights[0].pos", kLightPos);
    cube->mutable_material()->SetFloat("lights[0].constant", context->light_attenuation_constant(100));
    cube->mutable_material()->SetFloat("lights[0].linear", context->light_attenuation_linear(100));
    cube->mutable_material()->SetFloat("lights[0].quadratic", context->light_attenuation_quadratic(100));
    cube->mutable_material()->SetInt("light_count", 1);
  }

  camera_->mutable_transform()->SetTranslation(glm::vec3(-1.0, 1.5, 1.1));
  camera_->SetFarClip(200);
  context->PushCamera(camera_);

  skybox_.mutable_material()->PushShader(context->GetShader("skybox"));
  skybox_.mutable_material()->SetTexture("texture0", context->GetTexture("skybox"));
  skybox_.mutable_transform()->SetScale(glm::vec3(100, 100, 100));

  glEnable(GL_DEPTH_TEST);
}

void SkyboxScene::OnUpdate(Context *context)
{
  OnUpdateCommon _(context, "SkyboxScene");

  ImGui::SliderFloat3("light_scale", (float*)&light_scale_, 0, 2);
  ImGui::ColorEdit3("light_color", (float*)&light_color_);

  ImGui::Separator();

  ImGui::Text("camera_location %s", glm::to_string(context->camera().transform().translation()).c_str());
  ImGui::Text("camera_front %s", glm::to_string(context->camera().front()).c_str());

  for (int i = 0; i < cubes_.size(); ++i) {
    Cube* cube = &cubes_[i];
    cube->mutable_material()->SetVec3("material.ambient", context->material_property_ambient("gold"));
    cube->mutable_material()->SetVec3("material.diffuse", context->material_property_diffuse("gold"));
    cube->mutable_material()->SetVec3("material.specular", context->material_property_specular("gold"));
    cube->mutable_material()->SetFloat("material.shininess", context->material_property_shininess("gold"));
  }

  light_.OnUpdate(context);
  light_.mutable_transform()->SetScale(light_scale_);
  light_.mutable_material()->SetVec3("light_color", light_color_);

  for (int i = 0; i < cubes_.size(); ++i) {
    Cube* cube = &cubes_[i];
    cube->OnUpdate(context);
    cube->mutable_material()->SetVec3("lights[0].color", light_color_);
  }

  coord_.OnUpdate(context);
  skybox_.OnUpdate(context);
}

void SkyboxScene::OnRender(Context *context)
{
  for (int i = 0; i < cubes_.size(); ++i) {
    Cube* cube = &cubes_[i];
    cube->mutable_material()->SetVec3("view_pos", context->camera().transform().translation());
    cube->OnRender(context);
  }
  light_.OnRender(context);
  coord_.OnRender(context);
  skybox_.OnRender(context);
}

void SkyboxScene::OnExit(Context *context)
{
  for (int i = 0; i < cubes_.size(); ++i) {
    Cube* cube = &cubes_[i];
    cube->OnDestory(context);
  }
  light_.OnDestory(context);
  coord_.OnDestory(context);
  skybox_.OnDestory(context);
  context->PopCamera();
}