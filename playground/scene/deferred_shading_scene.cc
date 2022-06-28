#include "playground/scene/deferred_shading_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "renderer/framebuffer/gbuffer.h"
#include "renderer/transform.h"
#include "playground/scene/common.h"
#include "renderer/util.h"

void DeferredShadingScene::OnEnter(Scene *context)
{
  for (int i = 0; i < point_lights_num_; ++i) {
    point_lights_.push_back(PointLightObject());
    glm::vec3 point_light_pos(util::RandFromTo(-5, 5), util::RandFromTo(0, 5), util::RandFromTo(-5, 5));
    point_lights_[i].mutable_transform()->SetTranslation(point_light_pos);
    point_lights_[i].mutable_transform()->SetScale(glm::vec3(0.2, 0.2, 0.2));
    point_lights_[i].SetColor(glm::vec4(util::RandFromTo(0, 1), util::RandFromTo(0, 1), util::RandFromTo(0, 1), 1.0));
  }

  cube_transforms_.push_back(renderer::Transform(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)));
  cube_transforms_.push_back(renderer::Transform(glm::vec3(1, 1, 1), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)));
  cube_transforms_.push_back(renderer::Transform(glm::vec3(2, 2, 1), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)));
  cube_transforms_.push_back(renderer::Transform(glm::vec3(1, 2, 2), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)));
  for (int i = 0; i < cube_transforms_.size(); ++i) {
    cubes_.push_back(CubeObject());
    CubeObject* cube = &cubes_[i];
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

  renderer::GBuffer::Option option{context->framebuffer_size()};
  gbuffer_.Init(option);
  
  glEnable_(GL_DEPTH_TEST);
}

void DeferredShadingScene::OnUpdate(Scene *context)
{
  OnUpdateCommon _(context, "DeferredShadingScene");

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
    CubeObject* cube = &cubes_[i];
  }

  ImGui::Separator();

  ImGui::Text("Camera Type");
  ImGui::SameLine();
  if (ImGui::Button("Perceptive Camera")) {
    // context->SetCamera(nullptr);
  }
  ImGui::SameLine();
  if (ImGui::Button("Orthographic Camera")) {
    // context->mutable_camera()->SetType(renderer::Camera::Orthographic);
  }
  ImGui::SameLine();
  if (ImGui::Button("Orthographic Direction Light")) {
    // context->SetCamera(directional_light_.Test_GetCamera());
  }

  for (int i = 0; i < point_lights_num_; ++i) {
    point_lights_[i].OnUpdate(context);
  }

  for (int i = 0; i < cubes_.size(); ++i) {
    CubeObject* cube = &cubes_[i];
    cube->OnUpdate(context);
  }

  coord_.OnUpdate(context);
  plane_.OnUpdate(context);
  directional_light_.OnUpdate(context);
}

void DeferredShadingScene::OnRender(Scene *context)
{
  RenderShadowMap(context);

  // glm::mat4 shadow_map_vp = directional_light_.GetShadowMapVP();
  // renderer::Texture shadow_map_texture = directional_light_.GetShadowMapTexture();

  // ForwardShading(context, shadow_map_vp, shadow_map_texture);
  // DeferredShading(context, shadow_map_vp, shadow_map_texture);
}

void DeferredShadingScene::RenderShadowMap(Scene* context) {
  for (int i = 0; i < cubes_.size(); ++i) {
    CubeObject* cube = &cubes_[i];
    cube->OnRender(context);
  }
  plane_.OnRender(context);
}

void DeferredShadingScene::ForwardShading(Scene* context, const glm::mat4& shadow_map_vp,
                                          const renderer::Texture& shadow_map_texture) {
  for (int i = 0; i < cubes_.size(); ++i) {
    CubeObject* cube = &cubes_[i];
    cube->OnRender(context);
  }
  for (int i = 0; i < point_lights_num_; ++i) {
    point_lights_[i].OnRender(context);
  }
  coord_.OnRender(context);
  plane_.OnRender(context);
  directional_light_.OnRender(context);
}

void DeferredShadingScene::DeferredShading(Scene* context, const glm::mat4& shadow_map_vp,
                                           const renderer::Texture& shadow_map_texture) {
  gbuffer_.Bind();
  renderer::ShaderProgram deferred_shading_geometry = context->GetShader("deferred_shading_geometry");
  for (int i = 0; i < cubes_.size(); ++i) {
    CubeObject* cube = &cubes_[i];
    cube->OnRender(context);
  }
  plane_.OnRender(context);

  gbuffer_.Unbind();

  renderer::Texture texture_position = gbuffer_.GetTexture("position");
  renderer::Texture texture_normal = gbuffer_.GetTexture("normal");
  renderer::Texture texture_texcoord = gbuffer_.GetTexture("texcoord");
  renderer::Texture texture_frag_world_pos = gbuffer_.GetTexture("frag_world_pos");

  deferred_shading_quad_.OnRender(context);

  gbuffer_.BlitDepth(nullptr);

  for (int i = 0; i < point_lights_num_; ++i) {
    point_lights_[i].OnRender(context);
  }
}

void DeferredShadingScene::OnExit(Scene *context)
{
  for (int i = 0; i < cubes_.size(); ++i) {
    CubeObject* cube = &cubes_[i];
    cube->OnDestory(context);
  }
  for (int i = 0; i < point_lights_num_; ++i) {
    point_lights_[i].OnDestory(context);
  }
  coord_.OnDestory(context);
  plane_.OnDestory(context);

  context->SetCamera(nullptr);
}
