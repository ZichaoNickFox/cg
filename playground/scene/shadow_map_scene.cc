#include "playground/scene/shadow_map_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "engine/transform.h"
#include "playground/scene/common.h"

void ShadowMapScene::OnEnter(Context *context)
{
  engine::Transform light_transform(light_pos_, glm::quat(glm::vec3(0, 0, 0)), light_scale_);
  light_.SetTransform(light_transform);
  engine::Material light_material;
  light_material.SetShader(context->mutable_shader_repo()->GetOrLoadShader("point_light"));
  light_.SetMaterial(light_material);

  // http://www.barradeau.com/nicoptere/dump/materials.html
  cube_transforms_.push_back(engine::Transform(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)));
  cube_transforms_.push_back(engine::Transform(glm::vec3(1, 1, 1), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)));
  cube_transforms_.push_back(engine::Transform(glm::vec3(2, 2, 1), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)));
  cube_transforms_.push_back(engine::Transform(glm::vec3(1, 2, 2), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)));
  for (int i = 0; i < cube_transforms_.size(); ++i) {
    cubes_.push_back(Cube());
    Cube* cube = &cubes_[i];
    cube->SetTransform(cube_transforms_[i]);
    engine::Material cube_material;
    cube_material.SetShader(context->mutable_shader_repo()->GetOrLoadShader("phong"));
    cube->SetMaterial(cube_material);
  }

  camera_->mutable_transform()->SetTranslation(glm::vec3(5.3, 4.3, -3.5));
  camera_->mutable_transform()->SetRotation(glm::quat(glm::vec3(2.7, 0.75, -3.1)));
  context->PushCamera(camera_);

  std::vector<glm::vec3> positions{glm::vec3(0, 0, 0), glm::vec3(2, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 2, 0),
                                   glm::vec3(0, 0, 0), glm::vec3(0, 0, 2)};
  std::vector<glm::vec3> colors{glm::vec3(1, 0, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0),
                                glm::vec3(0, 0, 1), glm::vec3(0, 0, 1)};
  coord_.SetData(context, {positions, colors, GL_LINES, 5});

  engine::Material plane_material;
  plane_material.SetShader(context->mutable_shader_repo()->GetOrLoadShader("phong"));
  plane_material.SetVec3("material.ambient", material_property_.ambient);
  plane_material.SetVec3("material.diffuse", material_property_.diffuse);
  plane_material.SetVec3("material.specular", material_property_.specular);
  plane_material.SetFloat("material.shininess", material_property_.shininess);
  plane_.SetMaterial(plane_material);
  engine::Transform plane_transform;
  plane_transform.SetTranslation(glm::vec3(0, -1, 0));
  plane_transform.SetScale(glm::vec3(10, 0, 10));
  plane_.SetTransform(plane_transform);

  glEnable(GL_DEPTH_TEST);
}

void ShadowMapScene::OnUpdate(Context *context)
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
  plane_.OnUpdate(context);
}

void ShadowMapScene::OnGui(Context *context)
{
  bool open = true;
  ImGui::Begin("ShadowMapScene", &open, ImGuiWindowFlags_AlwaysAutoResize);
  RenderGoToGallery(context);

  ImGui::Separator();

  ImGui::SliderFloat3("light_scale", (float*)&light_scale_, 0, 2);
  ImGui::ColorEdit3("light_color", (float*)&light_color_);

  ImGui::Separator();

  ImGui::Text("camera_location %s", glm::to_string(context->camera().transform().translation()).c_str());
  ImGui::Text("camera_front %s", glm::to_string(context->camera().front()).c_str());

  ImGui::SliderFloat3("cube0_location", (float*)cubes_[0].mutable_transform()->mutable_translation(), -20, 0);

  glm::vec3 axis_x = cubes_[0].transform().rotation() * glm::vec3(1, 0, 0);
  glm::vec3 axis_y = cubes_[0].transform().rotation() * glm::vec3(0, 1, 0);
  glm::vec3 axis_z = cubes_[0].transform().rotation() * glm::vec3(0, 0, 1);
  float angle_xyz[3] = {0};
  ImGui::SliderFloat3("cube0_rotate_x", angle_xyz, -5, 5);
  cubes_[0].mutable_transform()->Rotate(glm::angleAxis(angle_xyz[0], axis_x));
  cubes_[0].mutable_transform()->Rotate(glm::angleAxis(angle_xyz[1], axis_y));
  cubes_[0].mutable_transform()->Rotate(glm::angleAxis(angle_xyz[2], axis_z));

  ImGui::SliderFloat3("cube0_scale", (float*)cubes_[0].mutable_transform()->mutable_scale(), -2, 2);

  light_.mutable_material()->SetVec3("light_color", light_color_);
  light_.mutable_transform()->SetTranslation(light_pos_);


  for (int i = 0; i < cubes_.size(); ++i) {
    Cube* cube = &cubes_[i];
    cube->mutable_material()->SetVec3("light_pos", light_pos_);
    cube->mutable_material()->SetVec3("material.ambient", material_property_.ambient);
    cube->mutable_material()->SetVec3("material.diffuse", material_property_.diffuse);
    cube->mutable_material()->SetVec3("material.specular", material_property_.specular);
    cube->mutable_material()->SetFloat("material.shininess", material_property_.shininess);
  }

  plane_.mutable_material()->SetVec3("light_pos", light_pos_);
  plane_.mutable_material()->SetVec3("light_color", light_color_);

  ImGui::Separator();

  ImGui::Text("Camera Type");
  ImGui::SameLine();
  if (ImGui::Button("Perceptive")) {
    camera_->SetType(engine::Camera::Perspective);
  }
  ImGui::SameLine();
  if (ImGui::Button("Orthographic")) {
    camera_->SetType(engine::Camera::Orthographic);
  }

  ImGui::End();
}

void ShadowMapScene::OnRender(Context *context)
{
  // directional_light_.ShadowMapRenderBegin(context);
  // RenderOnce(context);
  // directional_light_.ShadowMapRenderEnd(context);
  // engine::Texture texture = directional_light_.GetShadowMap();
  // engine::texture::SaveTexture2D("aaa", texture->id());

  RenderOnce(context);
}

void ShadowMapScene::RenderOnce(Context* context) {
  for (int i = 0; i < cubes_.size(); ++i) {
    Cube* cube = &cubes_[i];
    cube->mutable_material()->SetVec3("view_pos", context->camera().transform().translation());
    cube->OnRender(context);
  }
  plane_.mutable_material()->SetVec3("view_pos", context->camera().transform().translation());
  light_.OnRender(context);
  coord_.OnRender(context);
  plane_.OnRender(context);
}

void ShadowMapScene::OnExit(Context *context)
{
  for (int i = 0; i < cubes_.size(); ++i) {
    Cube* cube = &cubes_[i];
    cube->OnDestory(context);
  }
  light_.OnDestory(context);
  coord_.OnDestory(context);
  plane_.OnDestory(context);

  context->PopCamera();
}