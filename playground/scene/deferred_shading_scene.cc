#include "playground/scene/deferred_shading_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "engine/g_buffer.h"
#include "engine/transform.h"
#include "playground/scene/common.h"
#include "playground/util.h"

void DeferredShadingScene::OnEnter(Context *context)
{
  for (int i = 0; i < point_lights_num_; ++i) {
    point_lights_.push_back(PointLight());
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
    cubes_.push_back(Cube());
    Cube* cube = &cubes_[i];
    cube->SetTransform(cube_transforms_[i]);
    cube->mutable_material()->SetShader(context->GetShader("phong_shadow"));
  }

  camera_->mutable_transform()->SetTranslation(glm::vec3(5.3, 4.3, -3.5));
  camera_->mutable_transform()->SetRotation(glm::quat(glm::vec3(2.7, 0.75, -3.1)));
  context->SetCamera(camera_);

  plane_.mutable_material()->SetShader(context->GetShader("phong_shadow"));
  plane_.mutable_material()->SetVec3("material.ambient", context->material_property_ambient(material_name_));
  plane_.mutable_material()->SetVec3("material.diffuse", context->material_property_diffuse(material_name_));
  plane_.mutable_material()->SetVec3("material.specular", context->material_property_specular(material_name_));
  plane_.mutable_material()->SetFloat("material.shininess", context->material_property_shininess(material_name_));

  plane_.mutable_transform()->SetTranslation(glm::vec3(0, -1, 0));
  plane_.mutable_transform()->SetScale(glm::vec3(10, 0, 10));

  directional_light_.Init(context);
  directional_light_.mutable_transform()->SetTranslation(glm::vec3(-5, 6.3, -4.6));
  directional_light_.mutable_transform()->SetRotation(glm::quat(glm::vec3(2.48, -0.82, -3.09)));

  engine::GBuffer::Option option{context->frame_buffer_size()};
  g_buffer_.Init(option);
  
  glEnable(GL_DEPTH_TEST);

/*
  deferred_shading_quad_.mutable_material()->SetShader(context->GetShader("deferred_shading_lighting"));
  deferred_shading_quad_.mutable_material()->SetInt("light_count", point_lights_num_);
  deferred_shading_quad_.mutable_material()->SetVec3("clear_color", context->clear_color());
  for (int i = 0; i < point_lights_num_; ++i) {
    deferred_shading_quad_.mutable_material()->SetVec3(util::Format("lights[{}].color", i).c_str(),
                                      point_lights_[i].color());
    deferred_shading_quad_.mutable_material()->SetVec3(util::Format("lights[{}].pos", i).c_str(),
                                      point_lights_[i].transform().translation());
    deferred_shading_quad_.mutable_material()->SetFloat(util::Format("lights[{}].constant", i).c_str(),
                                        point_lights_[i].attenuation_constant());
    deferred_shading_quad_.mutable_material()->SetFloat(util::Format("lights[{}].linear", i).c_str(),
                                        point_lights_[i].attenuation_linear());
    deferred_shading_quad_.mutable_material()->SetFloat(util::Format("lights[{}].quadratic", i).c_str(),
                                        point_lights_[i].attenuation_quadratic());
  }
  deferred_shading_quad_.mutable_material()->SetVec3("material.ambient",
                                                     context->material_property_ambient(material_name_));
  deferred_shading_quad_.mutable_material()->SetVec3("material.diffuse",
                                                     context->material_property_diffuse(material_name_));
  deferred_shading_quad_.mutable_material()->SetVec3("material.specular",
                                                     context->material_property_specular(material_name_));
  deferred_shading_quad_.mutable_material()->SetFloat("material.shininess",
                                                      context->material_property_shininess(material_name_));
*/
}

void DeferredShadingScene::OnUpdate(Context *context)
{
  OnUpdateCommon _(context, "DeferredShadingScene");

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
    cube->mutable_material()->SetVec3("material.ambient", context->material_property_ambient(material_name_));
    cube->mutable_material()->SetVec3("material.diffuse", context->material_property_diffuse(material_name_));
    cube->mutable_material()->SetVec3("material.specular", context->material_property_specular(material_name_));
    cube->mutable_material()->SetFloat("material.shininess", context->material_property_shininess(material_name_));
  }

  ImGui::Separator();

  ImGui::Text("Camera Type");
  ImGui::SameLine();
  if (ImGui::Button("Perceptive Camera")) {
    // context->SetCamera(nullptr);
  }
  ImGui::SameLine();
  if (ImGui::Button("Orthographic Camera")) {
    // context->mutable_camera()->SetType(engine::Camera::Orthographic);
  }
  ImGui::SameLine();
  if (ImGui::Button("Orthographic Direction Light")) {
    // context->SetCamera(directional_light_.Test_GetCamera());
  }

  for (int i = 0; i < point_lights_num_; ++i) {
    point_lights_[i].OnUpdate(context);
  }

  for (int i = 0; i < cubes_.size(); ++i) {
    Cube* cube = &cubes_[i];
    cube->OnUpdate(context);
    /*
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
    */
  }

  coord_.OnUpdate(context);
  plane_.OnUpdate(context);
  directional_light_.OnUpdate(context);
}

void DeferredShadingScene::OnRender(Context *context)
{
  RenderShadowMap(context);

  // glm::mat4 shadow_map_vp = directional_light_.GetShadowMapVP();
  // engine::Texture shadow_map_texture = directional_light_.GetShadowMapTexture();

  // ForwardShading(context, shadow_map_vp, shadow_map_texture);
  // DeferredShading(context, shadow_map_vp, shadow_map_texture);
}

void DeferredShadingScene::RenderShadowMap(Context* context) {
  for (int i = 0; i < cubes_.size(); ++i) {
    Cube* cube = &cubes_[i];
    cube->OnRender(context);
  }
  plane_.OnRender(context);
}

void DeferredShadingScene::ForwardShading(Context* context, const glm::mat4& shadow_map_vp,
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

void DeferredShadingScene::DeferredShading(Context* context, const glm::mat4& shadow_map_vp,
                                           const engine::Texture& shadow_map_texture) {
  g_buffer_.Bind();
  engine::Shader deferred_shading_geometry = context->GetShader("deferred_shading_geometry");
  for (int i = 0; i < cubes_.size(); ++i) {
    Cube* cube = &cubes_[i];
    cube->mutable_material()->SetShader(deferred_shading_geometry);
    cube->OnRender(context);
  }
  plane_.mutable_material()->SetShader(deferred_shading_geometry);
  plane_.OnRender(context);

  g_buffer_.Unbind();

  engine::Texture texture_position = g_buffer_.GetTexture("position");
  engine::Texture texture_normal = g_buffer_.GetTexture("normal");
  engine::Texture texture_texcoord = g_buffer_.GetTexture("texcoord");
  engine::Texture texture_frag_world_pos = g_buffer_.GetTexture("frag_world_pos");

  deferred_shading_quad_.mutable_material()->SetTexture("texture_normal", texture_normal);
  deferred_shading_quad_.mutable_material()->SetTexture("texture_frag_world_pos", texture_frag_world_pos);
  deferred_shading_quad_.OnRender(context);

  g_buffer_.BlitDepth(nullptr);

  for (int i = 0; i < point_lights_num_; ++i) {
    point_lights_[i].OnRender(context);
  }
}

void DeferredShadingScene::OnExit(Context *context)
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
