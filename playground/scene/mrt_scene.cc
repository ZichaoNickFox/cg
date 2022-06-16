#include "playground/scene/mrt_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>
#include <vector>

#include "engine/repo/texture_repo.h"
#include "engine/transform.h"
#include "engine/util.h"
#include "playground/scene/common.h"
#include "playground/shaders.h"

void MrtScene::OnEnter(Context *context)
{
  engine::ColorFramebuffer::Option option{context->framebuffer_size(), 2, context->clear_color()};
  mrt_framebuffer_.Init(option);
  
  for (int i = 0; i < point_lights_num_; ++i) {
    point_lights_.push_back(PointLightObject());
    glm::vec3 point_light_pos(util::RandFromTo(-5, 5), util::RandFromTo(0, 5), util::RandFromTo(-5, 5));
    point_lights_[i].mutable_transform()->SetTranslation(point_light_pos);
    point_lights_[i].mutable_transform()->SetScale(glm::vec3(0.2, 0.2, 0.2));
    point_lights_[i].mutable_material()->SetShader(context->GetShader("point_light"));
    point_lights_[i].SetColor(glm::vec4(util::RandFromTo(0, 1), util::RandFromTo(0, 1), util::RandFromTo(0, 1), 1.0));
  }

  cube_transforms_.push_back(engine::Transform(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)));
  cube_transforms_.push_back(engine::Transform(glm::vec3(1, 1, 1), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)));
  cube_transforms_.push_back(engine::Transform(glm::vec3(2, 2, 1), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)));
  cube_transforms_.push_back(engine::Transform(glm::vec3(1, 2, 2), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1)));
  for (int i = 0; i < cube_transforms_.size(); ++i) {
    cubes_.push_back(CubeObject());
    CubeObject* cube = &cubes_[i];
    cube->SetTransform(cube_transforms_[i]);
    cube->mutable_material()->SetShader(context->GetShader("forward_shading"));
  }

  camera_->mutable_transform()->SetTranslation(glm::vec3(5.3, 4.3, -3.5));
  camera_->mutable_transform()->SetRotation(glm::quat(glm::vec3(2.7, 0.75, -3.1)));
  context->SetCamera(camera_.get());

  plane_.mutable_material()->SetShader(context->GetShader("forward_shading"));
  plane_.mutable_material()->SetVec3("material.ambient", material_property_.ambient);
  plane_.mutable_material()->SetVec3("material.diffuse", material_property_.diffuse);
  plane_.mutable_material()->SetVec3("material.specular", material_property_.specular);
  plane_.mutable_material()->SetFloat("material.shininess", material_property_.shininess);

  plane_.mutable_transform()->SetTranslation(glm::vec3(0, -1, 0));
  plane_.mutable_transform()->SetScale(glm::vec3(10, 0, 10));

  directional_light_.Init(context);
  directional_light_.mutable_transform()->SetTranslation(glm::vec3(-5, 6.3, -4.6));
  directional_light_.mutable_transform()->SetRotation(glm::quat(glm::vec3(2.48, -0.82, -3.09)));

  glEnable_(GL_DEPTH_TEST);
}

void MrtScene::OnUpdate(Context *context)
{
  OnUpdateCommon _(context, "MrtScene");

  for (int i = 0; i < point_lights_num_; ++i) {
    point_lights_[i].OnUpdate(context);
  }

  for (int i = 0; i < cubes_.size(); ++i) {
    CubeObject* cube = &cubes_[i];
    cube->OnUpdate(context);
    // cube->mutable_material()->SetInt("light_count", point_lights_num_);
    // for (int i = 0; i < point_lights_num_; ++i) {
    //   cube->mutable_material()->SetVec3(util::Format("lights[{}].color", i).c_str(),
    //                                     point_lights_[i].color());
    //   cube->mutable_material()->SetVec3(util::Format("lights[{}].pos", i).c_str(),
    //                                     point_lights_[i].transform().translation());
    //   cube->mutable_material()->SetFloat(util::Format("lights[{}].constant", i).c_str(),
    //                                      point_lights_[i].attenuation_constant());
    //   cube->mutable_material()->SetFloat(util::Format("lights[{}].linear", i).c_str(),
    //                                      point_lights_[i].attenuation_linear());
    //   cube->mutable_material()->SetFloat(util::Format("lights[{}].quadratic", i).c_str(),
    //                                      point_lights_[i].attenuation_quadratic());
    // }
  }

  coord_.OnUpdate(context);
  plane_.OnUpdate(context);
  directional_light_.OnUpdate(context);
}

void MrtScene::OnRender(Context *context)
{
  RenderShadowMap(context);

  // glm::mat4 shadow_map_vp = directional_light_.GetShadowMapVP();
  // engine::Texture shadow_map_texture = directional_light_.GetShadowMapTexture();
  // RenderScene(context, shadow_map_vp, shadow_map_texture);
}

void MrtScene::RenderShadowMap(Context* context) {
  // directional_light_.ShadowMappingPassBegin(context);
  for (int i = 0; i < cubes_.size(); ++i) {
    CubeObject* cube = &cubes_[i];
    cube->mutable_material()->SetShader(context->GetShader("shadow_map"));
    cube->OnRender(context);
  }
  plane_.mutable_material()->SetShader(context->GetShader("shadow_map"));
  plane_.OnRender(context);
  // directional_light_.ShadowMappingPassEnd(context);
}

void MrtScene::RenderScene(Context* context, const glm::mat4& shadow_map_vp,
                           const engine::Texture& shadow_map_texture) {
  mrt_framebuffer_.Bind();
  for (int i = 0; i < cubes_.size(); ++i) {
    CubeObject* cube = &cubes_[i];
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
  mrt_framebuffer_.Unbind();

  EmptyObject fullscreen_quad;
  fullscreen_quad.mutable_material()->SetShader(context->GetShader("mrt_fusion"));
  fullscreen_quad.mutable_material()->SetTexture("scene", mrt_framebuffer_.GetColorTexture(0));
  fullscreen_quad.mutable_material()->SetTexture("bright", mrt_framebuffer_.GetColorTexture(1));
  fullscreen_quad.OnRender(context);
}

void MrtScene::OnExit(Context *context)
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