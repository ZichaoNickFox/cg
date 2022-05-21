#include "playground/scene/SSAO_scene.h"

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

void SSAOScene::OnEnter(Context *context)
{
  for (int i = 0; i < point_lights_num_; ++i) {
    point_lights_.push_back(PointLight());
    glm::vec3 point_light_pos(util::RandFromTo(-5, 5), util::RandFromTo(0, 5), util::RandFromTo(-5, 5));
    point_lights_[i].mutable_transform()->SetTranslation(point_light_pos);
    point_lights_[i].mutable_transform()->SetScale(glm::vec3(0.2, 0.2, 0.2));
    glm::vec4 color(util::RandFromTo(0, 1), util::RandFromTo(0, 1), util::RandFromTo(0, 1), 1.0);
    point_lights_[i].SetColor(color);
  }

  camera_->mutable_transform()->SetTranslation(glm::vec3(1.78, 0.47, -2.30));
  camera_->mutable_transform()->SetRotation(glm::quat(-0.66, 0.19, -0.70, -0.18));
  context->SetCamera(camera_.get());

  plane_.mutable_transform()->SetTranslation(glm::vec3(0, -1, 0));
  plane_.mutable_transform()->SetScale(glm::vec3(10, 0, 10));

  nanosuit_.Init(context, "nanosuit", "nanosuit");
  for (int i = 0; i < nanosuit_.model_part_num(); ++i) {
    glm::quat rotation = glm::angleAxis(-float(M_PI) / 2, glm::vec3(1, 0, 0));
    nanosuit_.mutable_model_part(i)->mutable_transform()->SetRotation(rotation);
    nanosuit_.mutable_model_part(i)->mutable_transform()->SetScale(glm::vec3(0.2, 0.2, 0.2));
    nanosuit_.mutable_model_part(i)->mutable_transform()->SetTranslation(glm::vec3(0, -0.5, 0));
  }
  
  glEnable_(GL_DEPTH_TEST);

  SetupBufferAndPass(context);

  std::vector<glm::vec3> SSAO_noice(16);
  for (int i = 0; i < 16; ++i) {
     glm::vec3 noice(util::RandFromTo(-1, 1), util::RandFromTo(-1, 1), 0.0);
     SSAO_noice[i] = noice;
  }
  engine::CreateTexture2DParam param{1, 4, 4, std::vector<void*>{static_cast<void*>(SSAO_noice.data())}, GL_RGB32F};
  texture_SSAO_noice_ = context->mutable_texture_repo()->CreateTempTexture2D(param);
}

void SSAOScene::OnUpdate(Context *context)
{
  OnUpdateCommon _(context, "SSAOScene");

  ImGui::Separator();

  ImGui::Text("Camera Type");
  ImGui::SameLine();
  if (ImGui::Button("Perceptive Camera")) {
    context->SetCamera(camera_.get());
    camera_->SetType(engine::Camera::Perspective);
  }
  ImGui::SameLine();
  if (ImGui::Button("Orthographic Camera")) {
    context->SetCamera(camera_.get());
    camera_->SetType(engine::Camera::Orthographic);
  }

  for (int i = 0; i < point_lights_num_; ++i) {
    point_lights_[i].OnUpdate(context);
  }

  coord_.OnUpdate(context);
  plane_.OnUpdate(context);
}

void SSAOScene::SetupBufferAndPass(Context* context) {
  glm::ivec2 screen_size = context->io().screen_size();
  g_buffer_.Init({screen_size, {engine::kAttachmentPosition, engine::kAttachmentNormal,
                                engine::kAttachmentColor}});
  SSAO_buffer_.Init({screen_size, {engine::kAttachmentColor, engine::kAttachmentDepth}});
  blur_buffer_.Init({screen_size, {engine::kAttachmentColor, engine::kAttachmentDepth}});
  lighting_buffer_.Init({screen_size, {engine::kAttachmentColor, engine::kAttachmentDepth}});

  g_buffer_pass_.Init(&g_buffer_);
  SSAO_pass_.Init(&g_buffer_, &SSAO_buffer_);
  blur_pass_.Init(&SSAO_buffer_, &blur_buffer_);
  lighting_pass_.Init(&blur_buffer_, &lighting_buffer_);
}

void SSAOScene::OnRender(Context *context)
{
  RunGBufferPass(context, &g_buffer_pass_);
  
  EmptyObject quad;
  engine::Texture texture = g_buffer_pass_.g_buffer()->GetTexture(engine::kAttachmentNameColor);
  FullscreenQuadShader({texture_SSAO_noice_}, context, &quad);
  quad.OnRender(context);

  RunSSAOPass(context, &SSAO_pass_);
  RunBlurPass(context, &blur_pass_);
  RunLightingPass(context, &lighting_pass_);
}

void SSAOScene::RunForwardPass_Deprecated(Context* context, engine::ForwardPass* forward_pass) {
  forward_pass->Begin();

  const engine::MaterialProperty& material_property = engine::kMaterialProperties.at("gold");
  PhongShader::Param phong{material_property.ambient, material_property.diffuse,
                           material_property.specular, material_property.shininess};
  phong.scene_light_info = AsSceneLightInfo(point_lights_);
  for (int i = 0; i < point_lights_num_; ++i) {
    ColorShader({point_lights_[i].color()}, context, &point_lights_[i]);
    point_lights_[i].OnRender(context);
  }

  LinesShader({}, context, &coord_);
  coord_.OnRender(context);

  PhongShader(&phong, context, &plane_);
  plane_.OnRender(context);

  for (int i = 0; i < nanosuit_.model_part_num(); ++i) {
    ModelPart* model_part = nanosuit_.mutable_model_part(i);
    PhongShader(&phong, context, model_part);
    model_part->OnRender(context);
  }

  forward_pass->End();
}

void SSAOScene::RunGBufferPass(Context* context, engine::GBufferPass* g_buffer_pass) {
  g_buffer_pass->Begin();

  SSAOShader(SSAOShader::ParamGBuffer(), context, &plane_);
  plane_.OnRender(context);

  for (int i = 0; i < nanosuit_.model_part_num(); ++i) {
    ModelPart* model_part = nanosuit_.mutable_model_part(i);
    SSAOShader(SSAOShader::ParamGBuffer(), context, model_part);
    model_part->OnRender(context);
  }

  g_buffer_pass->End();
}

void SSAOScene::RunSSAOPass(Context* context, engine::SSAOPass* SSAO_pass) {
}

void SSAOScene::RunBlurPass(Context* context, engine::BlurPass* blue_pass) {
}

void SSAOScene::RunLightingPass(Context* context, engine::LightingPass* lighting_pass) {
}

void SSAOScene::OnExit(Context *context)
{
  for (int i = 0; i < point_lights_num_; ++i) {
    point_lights_[i].OnDestory(context);
  }
  coord_.OnDestory(context);
  plane_.OnDestory(context);

  context->SetCamera(nullptr);
}