#include "playground/scene/mrt_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>
#include <vector>

#include "renderer/shader.h"
#include "renderer/texture.h"
#include "renderer/transform.h"
#include "renderer/util.h"
#include "playground/scene/common.h"

void MrtScene::OnEnter(Scene *context)
{
  renderer::ColorFramebuffer::Option option{context->framebuffer_size(), 2, context->clear_color()};
  mrt_framebuffer_.Init(option);
  
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

  glEnable_(GL_DEPTH_TEST);
}

void MrtScene::OnUpdate(Scene *context)
{
  OnUpdateCommon _(context, "MrtScene");

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

void MrtScene::OnRender(Scene *context)
{
  RenderShadowMap(context);

  // glm::mat4 shadow_map_vp = directional_light_.GetShadowMapVP();
  // renderer::Texture shadow_map_texture = directional_light_.GetShadowMapTexture();
  // RenderScene(context, shadow_map_vp, shadow_map_texture);
}

void MrtScene::RenderShadowMap(Scene* context) {
  // directional_light_.ShadowMappingPassBegin(context);
  for (int i = 0; i < cubes_.size(); ++i) {
    CubeObject* cube = &cubes_[i];
    cube->OnRender(context);
  }
  plane_.OnRender(context);
  // directional_light_.ShadowMappingPassEnd(context);
}

void MrtScene::RenderScene(Scene* context, const glm::mat4& shadow_map_vp,
                           const renderer::Texture& shadow_map_texture) {
  mrt_framebuffer_.Bind();
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
  mrt_framebuffer_.Unbind();

  EmptyObject fullscreen_quad;
  fullscreen_quad.OnRender(context);
}

void MrtScene::OnExit(Scene *context)
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