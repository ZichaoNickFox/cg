#include "playground/scene/forward_shading_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

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
    point_lights_[i].mutable_material()->PushShader(context->GetShader("point_light"));
    point_lights_[i].SetColor(glm::vec4(util::RandFromTo(0, 1), util::RandFromTo(0, 1), util::RandFromTo(0, 1), 1.0));
  }

  // http://www.barradeau.com/nicoptere/dump/materials.html
  cube_transforms_.push_back(engine::Transform(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)));
  cube_transforms_.push_back(engine::Transform(glm::vec3(1, 1, 1), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)));
  cube_transforms_.push_back(engine::Transform(glm::vec3(2, 2, 1), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)));
  cube_transforms_.push_back(engine::Transform(glm::vec3(1, 2, 2), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)));
  for (int i = 0; i < cube_transforms_.size(); ++i) {
    cubes_.push_back(Cube());
    Cube* cube = &cubes_[i];
    cube->SetTransform(cube_transforms_[i]);
    cube->mutable_material()->PushShader(context->GetShader("forward_shading"));
  }

  camera_->mutable_transform()->SetTranslation(glm::vec3(5.3, 4.3, -3.5));
  camera_->mutable_transform()->SetRotation(glm::quat(glm::vec3(2.7, 0.75, -3.1)));
  context->PushCamera(camera_);

  std::vector<glm::vec3> positions{glm::vec3(0, 0, 0), glm::vec3(2, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 2, 0),
                                   glm::vec3(0, 0, 0), glm::vec3(0, 0, 2)};
  std::vector<glm::vec3> colors{glm::vec3(1, 0, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0),
                                glm::vec3(0, 0, 1), glm::vec3(0, 0, 1)};
  coord_.SetData(context, {positions, colors, GL_LINES, 5});

  plane_.mutable_material()->PushShader(context->GetShader("forward_shading"));
  plane_.mutable_material()->SetVec3("material.ambient", material_property_.ambient);
  plane_.mutable_material()->SetVec3("material.diffuse", material_property_.diffuse);
  plane_.mutable_material()->SetVec3("material.specular", material_property_.specular);
  plane_.mutable_material()->SetFloat("material.shininess", material_property_.shininess);

  plane_.mutable_transform()->SetTranslation(glm::vec3(0, -1, 0));
  plane_.mutable_transform()->SetScale(glm::vec3(10, 0, 10));

  directional_light_.Init(context);
  directional_light_.mutable_transform()->SetTranslation(glm::vec3(-5, 6.3, -4.6));
  directional_light_.mutable_transform()->SetRotation(glm::quat(glm::vec3(2.48, -0.82, -3.09)));
  
  glEnable(GL_DEPTH_TEST);
}

void ForwardShadingScene::OnUpdate(Context *context)
{
  ControlCameraByIo(context);

  for (int i = 0; i < point_lights_num_; ++i) {
    point_lights_[i].OnUpdate(context);
  }

  for (int i = 0; i < cubes_.size(); ++i) {
    Cube* cube = &cubes_[i];
    cube->OnUpdate(context);
    cube->mutable_material()->SetInt("light_count", point_lights_num_);
    for (int i = 0; i < point_lights_num_; ++i) {
      cube->mutable_material()->SetVec3(util::Format("lights[{}].color", i).c_str(),
                                        point_lights_[i].color());
      cube->mutable_material()->SetVec3(util::Format("lights[{}].pos", i).c_str(),
                                        point_lights_[i].transform().translation());
      cube->mutable_material()->SetFloat(util::Format("lights[{}].constant", i).c_str(),
                                         point_lights_[i].attenuation_constant());
      cube->mutable_material()->SetFloat(util::Format("lights[{}].linear", i).c_str(),
                                         point_lights_[i].attenuation_linear());
      cube->mutable_material()->SetFloat(util::Format("lights[{}].quadratic", i).c_str(),
                                         point_lights_[i].attenuation_quadratic());
    }
  }

  coord_.OnUpdate(context);
  plane_.OnUpdate(context);
  directional_light_.OnUpdate(context);
}

void ForwardShadingScene::OnGui(Context *context)
{
  bool open = true;
  ImGui::Begin("ForwardShadingScene", &open, ImGuiWindowFlags_AlwaysAutoResize);
  RenderFps(context);

  ImGui::Separator();

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

  for (int i = 0; i < cubes_.size(); ++i) {
    Cube* cube = &cubes_[i];
    cube->mutable_material()->SetVec3("material.ambient", material_property_.ambient);
    cube->mutable_material()->SetVec3("material.diffuse", material_property_.diffuse);
    cube->mutable_material()->SetVec3("material.specular", material_property_.specular);
    cube->mutable_material()->SetFloat("material.shininess", material_property_.shininess);
  }

  ImGui::Separator();

  ImGui::Text("Camera Type");
  ImGui::SameLine();
  if (ImGui::Button("Perceptive Camera")) {
    context->PopCamera();
  }
  ImGui::SameLine();
  if (ImGui::Button("Orthographic Camera")) {
    context->mutable_camera()->SetType(engine::Camera::Orthographic);
  }
  ImGui::SameLine();
  if (ImGui::Button("Orthographic Direction Light")) {
    context->PushCamera(directional_light_.Test_GetCamera());
  }

  ImGui::End();
}

void ForwardShadingScene::OnRender(Context *context)
{
  directional_light_.ShadowMappingPassBegin(context);
  RenderShadowMap(context);
  directional_light_.ShadowMappingPassEnd(context);

  glm::mat4 shadow_map_vp = directional_light_.GetShadowMapVP();
  engine::Texture shadow_map_texture = directional_light_.GetShadowMapTexture();
  RenderScene(context, shadow_map_vp, shadow_map_texture);
}

void ForwardShadingScene::RenderShadowMap(Context* context) {
  for (int i = 0; i < cubes_.size(); ++i) {
    Cube* cube = &cubes_[i];
    cube->mutable_material()->PushShader(context->GetShader("shadow_map"));
    cube->OnRender(context);
    cube->mutable_material()->PopShader();
  }
  plane_.mutable_material()->PushShader(context->GetShader("shadow_map"));
  plane_.OnRender(context);
  plane_.mutable_material()->PopShader();
}

void ForwardShadingScene::RenderScene(Context* context, const glm::mat4& shadow_map_vp,
                                       const engine::Texture& shadow_map_texture) {
  for (int i = 0; i < cubes_.size(); ++i) {
    Cube* cube = &cubes_[i];
    cube->mutable_material()->SetTexture("shadow_map_texture", shadow_map_texture);
    cube->mutable_material()->SetMat4("shadow_map_vp", shadow_map_vp);
    cube->OnRender(context);
  }
  for (int i = 0; i < point_lights_num_; ++i) {
    point_lights_[i].OnRender(context);
  }
  coord_.OnRender(context);
  plane_.mutable_material()->SetTexture("shadow_map_texture", shadow_map_texture);
  plane_.mutable_material()->SetMat4("shadow_map_vp", shadow_map_vp);
  plane_.OnRender(context);
  directional_light_.OnRender(context);
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

  context->PopCamera();
}