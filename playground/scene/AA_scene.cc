#include "playground/scene/AA_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>
#include <vector>

#include "engine/transform.h"
#include "playground/scene/common.h"
#include "playground/texture_repo.h"
#include "playground/util.h"

void AAScene::OnEnter(Context *context)
{
  engine::MSFrameBuffer::Option ms_fbo_option{
      "mrt_2_ms_frame_buffer", context->screen_width(), context->screen_height(), 1,
      {context->clear_color()/*, glm::vec4(0, 0, 0, 1)*/}, 4};
  ms_frame_buffer_.Init(ms_fbo_option);

  engine::ColorFrameBuffer::Option color_fbo_option{
      "color_frame_buffer", context->screen_width(), context->screen_height(), 1,
      {context->clear_color()/*, glm::vec4(0, 0, 0, 1)*/}};
  color_frame_buffer_.Init(color_fbo_option);
  
  for (int i = 0; i < point_lights_num_; ++i) {
    point_lights_.push_back(PointLight());
    glm::vec3 point_light_pos(util::RandFromTo(-5, 5), util::RandFromTo(0, 5), util::RandFromTo(-5, 5));
    point_lights_[i].mutable_transform()->SetTranslation(point_light_pos);
    point_lights_[i].mutable_transform()->SetScale(glm::vec3(0.2, 0.2, 0.2));
    point_lights_[i].mutable_material()->PushShader(context->mutable_shader_repo()->GetOrLoadShader("point_light"));
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
    cube->mutable_material()->PushShader(context->mutable_shader_repo()->GetOrLoadShader("forward_shading"));
  }

  camera_->mutable_transform()->SetTranslation(glm::vec3(5.3, 4.3, -3.5));
  camera_->mutable_transform()->SetRotation(glm::quat(glm::vec3(2.7, 0.75, -3.1)));
  context->PushCamera(camera_);

  std::vector<glm::vec3> positions{glm::vec3(0, 0, 0), glm::vec3(2, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 2, 0),
                                   glm::vec3(0, 0, 0), glm::vec3(0, 0, 2)};
  std::vector<glm::vec3> colors{glm::vec3(1, 0, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0),
                                glm::vec3(0, 0, 1), glm::vec3(0, 0, 1)};
  coord_.SetData(context, {positions, colors, GL_LINES, 5});

  plane_.mutable_material()->PushShader(context->mutable_shader_repo()->GetOrLoadShader("forward_shading"));
  plane_.mutable_material()->SetVec3("material.ambient", material_property_.ambient);
  plane_.mutable_material()->SetVec3("material.diffuse", material_property_.diffuse);
  plane_.mutable_material()->SetVec3("material.specular", material_property_.specular);
  plane_.mutable_material()->SetFloat("material.shininess", material_property_.shininess);

  plane_.mutable_transform()->SetTranslation(glm::vec3(0, -1, 0));
  plane_.mutable_transform()->SetScale(glm::vec3(10, 0, 10));

  directional_light_.Init(context);
  directional_light_.mutable_transform()->SetTranslation(glm::vec3(-5, 6.3, -4.6));
  directional_light_.mutable_transform()->SetRotation(glm::quat(glm::vec3(2.48, -0.82, -3.09)));

  fullscreen_quad_.mutable_material()->PushShader(context->mutable_shader_repo()->GetOrLoadShader("mrt_fusion"));
  
  glEnable(GL_DEPTH_TEST);
}

void AAScene::OnUpdate(Context *context)
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

void AAScene::OnGui(Context *context)
{
  bool open = true;
  ImGui::Begin("AAScene", &open, ImGuiWindowFlags_AlwaysAutoResize);
  RenderFps(context);
  ImGui::End();
}

void AAScene::OnRender(Context *context)
{
  RenderShadowMap(context);

  glm::mat4 shadow_map_vp = directional_light_.GetShadowMapVP();
  engine::Texture shadow_map_texture = directional_light_.GetShadowMapTexture();
  RenderScene(context, shadow_map_vp, shadow_map_texture);
}

void AAScene::RenderShadowMap(Context* context) {
  directional_light_.ShadowMapRenderBegin(context);
  for (int i = 0; i < cubes_.size(); ++i) {
    Cube* cube = &cubes_[i];
    cube->mutable_material()->PushShader(context->mutable_shader_repo()->GetOrLoadShader("shadow_map"));
    cube->OnRender(context);
    cube->mutable_material()->PopShader();
  }
  plane_.mutable_material()->PushShader(context->mutable_shader_repo()->GetOrLoadShader("shadow_map"));
  plane_.OnRender(context);
  plane_.mutable_material()->PopShader();
  directional_light_.ShadowMapRenderEnd(context);
}

void AAScene::RenderScene(Context* context, const glm::mat4& shadow_map_vp,
                           const engine::Texture& shadow_map_texture) {
  ms_frame_buffer_.Bind();
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
  ms_frame_buffer_.Unbind();

  // ms_frame_buffer_.Blit(&color_frame_buffer_);

  fullscreen_quad_.mutable_material()->SetTexture("scene", ms_frame_buffer_.GetTexture(0));
  fullscreen_quad_.mutable_material()->SetTexture("bright", ms_frame_buffer_.GetTexture(1));
  // fullscreen_quad_.mutable_material()->SetTexture("scene", color_frame_buffer_.GetTexture(0));
  // fullscreen_quad_.mutable_material()->SetTexture("bright", color_frame_buffer_.GetTexture(1));
  fullscreen_quad_.OnRender(context);
}

void AAScene::OnExit(Context *context)
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