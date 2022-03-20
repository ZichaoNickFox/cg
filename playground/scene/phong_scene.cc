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
  const glm::vec3 kLightColor = glm::vec3(0.0, 1.0, 0.0);
  const glm::vec3 kLightPos = glm::vec3(1, 1, 0);
  const glm::vec3 kLightScale = glm::vec3(0.4, 0.4, 0.4);
  engine::Transform light_transform(kLightPos, {0, 0, 0}, kLightScale);
  light_.SetTransform(light_transform);
  engine::Material material;
  material.SetShader(context->mutable_shader_repo()->GetOrLoadShader("point_light"));
  material.SetVec3("light_color", kLightColor);
  light_.SetMaterial(material);

  // http://www.barradeau.com/nicoptere/dump/materials.html
  const glm::vec3 kCubePosition = glm::vec3(0, 0, 0);
  engine::Transform cube_transform;
  cube_transform.SetTranslation(kCubePosition);
  engine::Material cube_material;
  cube_material.SetShader(context->mutable_shader_repo()->GetOrLoadShader("phong"));
  cube_material.SetVec3("light_color", kLightColor);
  cube_material.SetVec3("light_pos", kLightPos);
  cube_.SetMaterial(cube_material);

  engine::Camera* camera = context->mutable_camera();
  camera->mutable_transform()->SetTranslation(glm::vec3(-1.0, 1.5, 1.1));
  camera->SetFront(glm::vec3(0.7, -0.4, -0.4));

  glEnable(GL_DEPTH_TEST);
}

void PhongScene::OnUpdate(Context *context)
{
  ControlCameraByIo(context);
  cube_.OnUpdate(context);
  light_.OnUpdate(context);

  light_.mutable_transform()->SetScale(light_scale_);
  light_.mutable_material()->SetVec3("light_color", light_color_);
  cube_.mutable_material()->SetVec3("light_color", light_color_);
}

void PhongScene::OnGui(Context *context)
{
  bool open = true;
  ImGui::Begin("PhongScene", &open, ImGuiWindowFlags_AlwaysAutoResize);
  RenderGoToGallery(context);

  ImGui::Separator();

  ImGui::SliderFloat3("light_scale", (float*)&light_scale_, 0, 2);
  ImGui::ColorEdit3("light_color", (float*)&light_color_);

  ImGui::Separator();

  ImGui::Text("camera_location %s", glm::to_string(context->camera().transform().translation()).c_str());
  ImGui::Text("camera_front %s", glm::to_string(context->camera().front()).c_str());

  if (ImGui::Button("gold")) {
    material_property_ = gold_;
  } else if (ImGui::Button("silver")) {
    material_property_ = silver_;
  } else if (ImGui::Button("jade")) {
    material_property_ = jade_;
  } else if (ImGui::Button("rube")) {
    material_property_ = ruby_;
  }
  cube_.mutable_material()->SetVec3("material.ambient", material_property_.ambient);
  cube_.mutable_material()->SetVec3("material.diffuse", material_property_.diffuse);
  cube_.mutable_material()->SetVec3("material.specular", material_property_.specular);
  cube_.mutable_material()->SetFloat("material.shininess", material_property_.shininess);

  ImGui::End();
}

void PhongScene::OnRender(Context *context)
{
  cube_.mutable_material()->SetVec3("view_pos", context->camera().transform().translation());
  cube_.OnRender(context);
  light_.OnRender(context);
}

void PhongScene::OnExit(Context *context)
{
  cube_.OnDestory(context);
  light_.OnDestory(context);
}