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
  engine::Material material;
  material.PushShader(context->mutable_shader_repo()->GetOrLoadShader("point_light"));
  material.SetVec3("light_color", kLightColor);
  light_.SetMaterial(material);

  // http://www.barradeau.com/nicoptere/dump/materials.html
  cube_positions_.push_back(glm::vec3(0, 0, 0));
  cube_positions_.push_back(glm::vec3(1, 1, 1));
  cube_positions_.push_back(glm::vec3(2, 2, 1));
  cube_positions_.push_back(glm::vec3(1, 2, 2));
  for (int i = 0; i < cube_positions_.size(); ++i) {
    cubes_.push_back(Cube());
    Cube* cube = &cubes_[i];
    engine::Transform cube_transform;
    cube_transform.SetTranslation(cube_positions_[i]);
    cube->SetTransform(cube_transform);
    engine::Material cube_material;
    cube_material.PushShader(context->mutable_shader_repo()->GetOrLoadShader("phong"));
    cube_material.SetVec3("light_color", kLightColor);
    cube_material.SetVec3("light_pos", kLightPos);
    cube->SetMaterial(cube_material);
  }

  camera_->mutable_transform()->SetTranslation(glm::vec3(-1.0, 1.5, 1.1));
  // camera_->SetFront(glm::vec3(0.7, -0.4, -0.4));
  context->PushCamera(camera_);

  std::vector<glm::vec3> positions{glm::vec3(0, 0, 0), glm::vec3(2, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 2, 0),
                                   glm::vec3(0, 0, 0), glm::vec3(0, 0, 2)};
  std::vector<glm::vec3> colors{glm::vec3(1, 0, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0),
                                glm::vec3(0, 0, 1), glm::vec3(0, 0, 1)};
  coord_.SetData(context, {positions, colors, GL_LINES, 5});

  engine::Material skybox_material;
  skybox_material.PushShader(context->mutable_shader_repo()->GetOrLoadShader("skybox"));
  skybox_material.SetTexture("texture0", context->mutable_texture_repo()->GetOrLoadTexture("skybox"));
  skybox_.SetMaterial(skybox_material);
  skybox_.mutable_transform()->SetScale(glm::vec3(200, 200, 200));

  glEnable(GL_DEPTH_TEST);
}

void SkyboxScene::OnUpdate(Context *context)
{
  ControlCameraByIo(context);
  light_.OnUpdate(context);
  light_.mutable_transform()->SetScale(light_scale_);
  light_.mutable_material()->SetVec3("light_color", light_color_);

  for (int i = 0; i < cubes_.size(); ++i) {
    Cube* cube = &cubes_[i];
    cube->OnUpdate(context);
    cube->mutable_material()->SetVec3("light_color", light_color_);
  }

  coord_.OnUpdate(context);
  skybox_.OnUpdate(context);
}

void SkyboxScene::OnGui(Context *context)
{
  bool open = true;
  ImGui::Begin("SkyScene", &open, ImGuiWindowFlags_AlwaysAutoResize);
  RenderGoToGallery(context);

  ImGui::Separator();

  ImGui::SliderFloat3("light_scale", (float*)&light_scale_, 0, 2);
  ImGui::ColorEdit3("light_color", (float*)&light_color_);

  ImGui::Separator();

  ImGui::Text("camera_location %s", glm::to_string(context->camera().transform().translation()).c_str());
  ImGui::Text("camera_front %s", glm::to_string(context->camera().front()).c_str());

  for (int i = 0; i < cubes_.size(); ++i) {
    Cube* cube = &cubes_[i];
    cube->mutable_material()->SetVec3("material.ambient", material_property_.ambient);
    cube->mutable_material()->SetVec3("material.diffuse", material_property_.diffuse);
    cube->mutable_material()->SetVec3("material.specular", material_property_.specular);
    cube->mutable_material()->SetFloat("material.shininess", material_property_.shininess);
  }

  ImGui::End();
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