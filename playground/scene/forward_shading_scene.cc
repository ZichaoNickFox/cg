#include "playground/scene/forward_shading_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "engine/constants.h"
#include "engine/framebuffer_attachment.h"
#include "engine/pass.h"
#include "engine/transform.h"
#include "engine/util.h"
#include "playground/object/empty_object.h"
#include "playground/scene/common.h"

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
  context->SetCamera(camera_.get());

  plane_.mutable_transform()->SetTranslation(glm::vec3(0, -1, 0));
  plane_.mutable_transform()->SetScale(glm::vec3(10, 0, 10));

  directional_light_.Init(context);
  directional_light_.mutable_transform()->SetTranslation(glm::vec3(-5, 6.3, -4.6));
  directional_light_.mutable_transform()->SetRotation(glm::quat(glm::vec3(2.48, -0.82, -3.09)));
  
  glEnable_(GL_DEPTH_TEST);

  light_camera_->SetTransform(directional_light_.transform());
  depth_framebuffer_.Init({context->framebuffer_size(), {engine::kAttachmentColor, engine::kAttachmentDepth}});
  depth_buffer_pass_.Init(&depth_framebuffer_, light_camera_);

  forward_framebuffer_.Init({context->framebuffer_size(), {engine::kAttachmentColor, engine::kAttachmentDepth}});
  forward_pass_.Init(&forward_framebuffer_);
}

void ForwardShadingScene::OnUpdate(Context *context)
{
  OnUpdateCommon _(context, "ForwardShadingScene");

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

  for (int i = 0; i < point_lights_num_; ++i) {
    point_lights_[i].OnUpdate(context);
  }

  coord_.OnUpdate(context);
  plane_.OnUpdate(context);
  directional_light_.OnUpdate(context);
}

void ForwardShadingScene::OnRender(Context *context)
{
  RunDepthBufferPass(context, &depth_buffer_pass_);

  forward_pass_.Update(depth_buffer_pass_.scene_shadow_info());
  RunForwardPass_Deprecated(context, &forward_pass_);

  EmptyObject quad;
  FullscreenQuadShader({forward_framebuffer_.GetTexture(engine::kAttachmentNameColor)}, context, &quad);
  quad.OnRender(context);
}

void ForwardShadingScene::RunDepthBufferPass(Context* context, engine::DepthBufferPass* depth_buffer_pass) {
  depth_buffer_pass->Begin();

  DepthBufferShader::Param param{depth_buffer_pass->camera(), context->GetShader("depth_buffer")};
  for (int i = 0; i < cubes_.size(); ++i) {
    Cube* cube = &cubes_[i];
    DepthBufferShader{param, cube};
    cube->OnRender(context);
  }
  DepthBufferShader{param, &plane_};
  plane_.OnRender(context);

  depth_buffer_pass->End();
}

void ForwardShadingScene::RunForwardPass_Deprecated(Context* context, engine::ForwardPass* forward_pass) {
  forward_pass->Begin();

  const engine::MaterialProperty& material_property = engine::kMaterialProperties.at("gold");
  PhongShader::Param phong{material_property.ambient, material_property.diffuse,
                           material_property.specular, material_property.shininess};
  phong.scene_shadow_info = forward_pass->prepass_shadow_info();
  phong.scene_light_info = AsSceneLightInfo(point_lights_);
  for (int i = 0; i < cubes_.size(); ++i) {
    Cube* cube = &cubes_[i];
    PhongShader(&phong, context, cube);
    cube->OnRender(context);
  }
  for (int i = 0; i < point_lights_num_; ++i) {
    ColorShader({point_lights_[i].color()}, context, &point_lights_[i]);
    point_lights_[i].OnRender(context);
  }

  LinesShader({}, context, &coord_);
  coord_.OnRender(context);

  PhongShader(&phong, context, &plane_);
  plane_.OnRender(context);

  LinesShader({}, context, directional_light_.mutable_lines());
  TextureShader({context->GetTexture("directional_light")}, context, directional_light_.mutable_billboard());
  directional_light_.OnRender(context);

  forward_pass->End();
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