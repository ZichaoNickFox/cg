#include "playground/scene/forward_shading_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "engine/pass.h"
#include "engine/transform.h"
#include "playground/scene/common.h"
#include "playground/util.h"

void ForwardShadingScene::OnEnter(Context *context)
{
  for (int i = 0; i < point_lights_num_; ++i) {
    point_lights_.push_back(PointLight());
    glm::vec3 point_light_pos(util::RandFromTo(-5, 5), util::RandFromTo(0, 5), util::RandFromTo(-5, 5));
    point_lights_[i].mutable_transform()->SetTranslation(point_light_pos);
    point_lights_[i].mutable_transform()->SetScale(glm::vec3(0.2, 0.2, 0.2));
    glm::vec4 color(util::RandFromTo(0, 1), util::RandFromTo(0, 1), util::RandFromTo(0, 1), 1.0);
    point_lights_[i].SetColor(color);
  }

  cube_transforms_.push_back(engine::Transform(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)));
  cube_transforms_.push_back(engine::Transform(glm::vec3(1, 1, 1), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)));
  cube_transforms_.push_back(engine::Transform(glm::vec3(2, 2, 1), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)));
  cube_transforms_.push_back(engine::Transform(glm::vec3(1, 2, 2), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)));
  for (int i = 0; i < cube_transforms_.size(); ++i) {
    cubes_.push_back(Cube());
    Cube* cube = &cubes_[i];
    cube->SetTransform(cube_transforms_[i]);
  }

  camera_->mutable_transform()->SetTranslation(glm::vec3(5.3, 4.3, -3.5));
  camera_->mutable_transform()->SetRotation(glm::quat(glm::vec3(2.7, 0.75, -3.1)));
  context->SetCamera(camera_);

  plane_.mutable_transform()->SetTranslation(glm::vec3(0, -1, 0));
  plane_.mutable_transform()->SetScale(glm::vec3(10, 0, 10));

  directional_light_.Init(context);
  directional_light_.mutable_transform()->SetTranslation(glm::vec3(-5, 6.3, -4.6));
  directional_light_.mutable_transform()->SetRotation(glm::quat(glm::vec3(2.48, -0.82, -3.09)));
  
  glEnable(GL_DEPTH_TEST);
}

void ForwardShadingScene::OnUpdate(Context *context)
{
  OnUpdateCommon _(context, "ForwardShadingScene");

  ImGui::Text("camera_location %s", glm::to_string(context->camera().transform().translation()).c_str());
  ImGui::Text("camera_front %s", glm::to_string(context->camera().front()).c_str());
  ImGui::Text("camera_euler %s", glm::to_string(glm::eulerAngles(context->camera().transform().rotation())).c_str());

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

  ImGui::Separator();

  ImGui::Text("Camera Type");
  ImGui::SameLine();
  if (ImGui::Button("Perceptive Camera")) {
    camera_->SetType(engine::Camera::Perspective);
  }
  ImGui::SameLine();
  if (ImGui::Button("Orthographic Camera")) {
    camera_->SetType(engine::Camera::Orthographic);
  }
  ImGui::SameLine();
  if (ImGui::Button("Orthographic Direction Light")) {
    // context->SetCamera(directional_light_.Test_GetCamera());
  }

  for (int i = 0; i < point_lights_num_; ++i) {
    point_lights_[i].OnUpdate(context);
  }

  coord_.OnUpdate(context);
  plane_.OnUpdate(context);
  directional_light_.OnUpdate(context);
}

void ForwardShadingScene::OnRender(Context *context)
{
  engine::ZBufferPass z_buffer_pass({"depth_buffer", context->io().screen_width(), context->io().screen_height()},
                                    directional_light_.transform());
  RunZBufferPass(context, &z_buffer_pass);

  engine::ForwardPass forward_pass;
  RunForwardPass(context, &forward_pass);

  engine::ShadowPass shadow_pass(z_buffer_pass.camera_vp(), z_buffer_pass.z_buffer_texture());
  RunShadowPass(context, &shadow_pass);
}

void ForwardShadingScene::RunZBufferPass(Context* context, engine::ZBufferPass* z_buffer_pass) {
  z_buffer_pass->Begin();

  for (int i = 0; i < cubes_.size(); ++i) {
    Cube* cube = &cubes_[i];
    ZBufferShader{context->GetShader("z_buffer"), z_buffer_pass->camera(), cube};
    cube->OnRender(context);
  }
  ZBufferShader{context->GetShader("z_buffer"), z_buffer_pass->camera(), &plane_};
  plane_.OnRender(context);

  z_buffer_pass->End();
}

void ForwardShadingScene::RunForwardPass(Context* context, engine::ForwardPass* forward_pass) {
  forward_pass->Begin();

  PhongShader::Param phong;
  phong.light_info = point_lights_;
  phong.ambient = context->material_property_ambient("gold");
  phong.diffuse = context->material_property_diffuse("gold");
  phong.specular = context->material_property_specular("gold");
  phong.shininess = context->material_property_shininess("gold");
  for (int i = 0; i < cubes_.size(); ++i) {
    Cube* cube = &cubes_[i];
    PhongShader(phong, context, cube);
    cube->OnRender(context);
  }
  for (int i = 0; i < point_lights_num_; ++i) {
    ColorShader({point_lights_[i].color()}, context, &point_lights_[i]);
    point_lights_[i].OnRender(context);
  }

  LinesShader({}, context, &coord_);
  coord_.OnRender(context);

  PhongShader(phong, context, &plane_);
  plane_.OnRender(context);

  LinesShader({}, context, directional_light_.mutable_lines());
  Texture0Shader({context->GetTexture("directional_light")}, context, directional_light_.mutable_billboard());
  directional_light_.OnRender(context);

  forward_pass->End();
}

void ForwardShadingScene::RunShadowPass(Context* context, engine::ShadowPass* shadow_pass) {
  shadow_pass->Begin();



  shadow_pass->End();
}

void ForwardShadingScene::OnExit(Context *context)
{
  for (int i = 0; i < cubes_.size(); ++i) {
    Cube* cube = &cubes_[i];
    cube->OnDestory(context);
  }
  for (int i = 0; i < point_lights_num_; ++i) {
    point_lights_[i].OnDestory(context);
  }
  coord_.OnDestory(context);
  plane_.OnDestory(context);

  context->SetCamera(nullptr);
}