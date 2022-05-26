#include "playground/scene/shadow_scene.h"

#include <glm/glm.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "engine/constants.h"
#include "engine/framebuffer_attachment.h"
#include "engine/math.h"
#include "engine/pass.h"
#include "engine/transform.h"
#include "engine/util.h"
#include "playground/object/empty_object.h"
#include "playground/scene/common.h"

void ShadowScene::OnEnter(Context *context)
{
  for (int i = 0; i < point_lights_num_; ++i) {
    point_lights_.push_back(PointLight());
    glm::vec3 point_light_pos(engine::RandFromTo(-5, 5), engine::RandFromTo(0, 5), engine::RandFromTo(-5, 5));
    point_lights_[i].mutable_transform()->SetTranslation(point_light_pos);
    point_lights_[i].mutable_transform()->SetScale(glm::vec3(0.2, 0.2, 0.2));
    glm::vec4 color(engine::RandFromTo(0, 1), engine::RandFromTo(0, 1), engine::RandFromTo(0, 1), 1.0);
    point_lights_[i].SetColor(color);
  }

  camera_->mutable_transform()->SetTranslation(glm::vec3(1.78, 0.47, -2.30));
  camera_->mutable_transform()->SetRotation(glm::quat(-0.66, 0.19, -0.70, -0.18));
  context->SetCamera(camera_.get());

  plane_.mutable_transform()->SetTranslation(glm::vec3(0, -1, 0));
  plane_.mutable_transform()->SetScale(glm::vec3(10, 0, 10));

  directional_light_.Init(context);
  directional_light_.mutable_transform()->SetTranslation(glm::vec3(-5, 6.3, -4.6));
  directional_light_.mutable_transform()->SetRotation(glm::quat(glm::vec3(2.48, -0.82, -3.09)));

  nanosuit_.Init(context, "nanosuit", "nanosuit");
  for (int i = 0; i < nanosuit_.model_part_num(); ++i) {
    glm::quat rotation = glm::angleAxis(-float(M_PI) / 2, glm::vec3(1, 0, 0));
    nanosuit_.mutable_model_part(i)->mutable_transform()->SetRotation(rotation);
    nanosuit_.mutable_model_part(i)->mutable_transform()->SetScale(glm::vec3(0.2, 0.2, 0.2));
    nanosuit_.mutable_model_part(i)->mutable_transform()->SetTranslation(glm::vec3(0, -0.5, 0));
  }
  
  glEnable_(GL_DEPTH_TEST);

  depth_framebuffer_.Init({context->framebuffer_size(), {engine::kAttachmentDepth}});
  depth_buffer_pass_.Init(&depth_framebuffer_, camera_);

  forward_framebuffer_.Init({context->framebuffer_size(), {engine::kAttachmentColor, engine::kAttachmentDepth}});
  forward_pass_.Init(&forward_framebuffer_);
}

void ShadowScene::OnUpdate(Context *context)
{
  OnUpdateCommon _(context, "SSAOScene");

  ImGui::Separator();

  ImGui::Text("Camera Type");
  ImGui::SameLine();

  for (int i = 0; i < point_lights_num_; ++i) {
    point_lights_[i].OnUpdate(context);
  }

  coord_.OnUpdate(context);
  plane_.OnUpdate(context);
  directional_light_.OnUpdate(context);
}

void ShadowScene::OnRender(Context *context)
{
  RunDepthBufferPass(context, &depth_buffer_pass_);

  forward_pass_.Update(depth_buffer_pass_.scene_shadow_info());
  RunForwardPass_Deprecated(context, &forward_pass_);

  EmptyObject quad;
  FullscreenQuadShader({forward_framebuffer_.GetTexture(engine::kAttachmentColor.name)}, context, &quad);
  quad.OnRender(context);
}

void ShadowScene::RunDepthBufferPass(Context* context, engine::DepthBufferPass* depth_buffer_pass) {
  depth_buffer_pass->Begin();

  DepthBufferShader::Param param{depth_buffer_pass->camera(), context->GetShader("depth_buffer")};
  DepthBufferShader{param, &plane_};
  plane_.OnRender(context);

  for (int i = 0; i < nanosuit_.model_part_num(); ++i) {
    ModelPart* model_part = nanosuit_.mutable_model_part(i);
    DepthBufferShader{param, model_part};
    model_part->OnRender(context);
  }

  depth_buffer_pass->End();
}

void ShadowScene::RunForwardPass_Deprecated(Context* context, engine::ForwardPass* forward_pass) {
  forward_pass->Begin();

  const engine::MaterialProperty& material_property = engine::kMaterialProperties.at("gold");
  PhongShader::Param phong{material_property.ambient, material_property.diffuse,
                           material_property.specular, material_property.shininess};
  phong.scene_shadow_info = forward_pass->prepass_shadow_info();
  phong.scene_light_info = AsSceneLightInfo(point_lights_);
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

  for (int i = 0; i < nanosuit_.model_part_num(); ++i) {
    ModelPart* model_part = nanosuit_.mutable_model_part(i);
    PhongShader(&phong, context, model_part);
    model_part->OnRender(context);
  }

  forward_pass->End();
}

void ShadowScene::OnExit(Context *context)
{
  for (int i = 0; i < point_lights_num_; ++i) {
    point_lights_[i].OnDestory(context);
  }
  coord_.OnDestory(context);
  plane_.OnDestory(context);

  context->SetCamera(nullptr);
}
