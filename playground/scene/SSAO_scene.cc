#include "playground/scene/SSAO_scene.h"

#include <glm/glm.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "renderer/framebuffer_attachment.h"
#include "renderer/math.h"
#include "renderer/pass.h"
#include "renderer/scene_common.h"
#include "renderer/transform.h"
#include "renderer/util.h"

void SSAOScene::OnEnter(Scene *context)
{
  for (int i = 0; i < point_lights_num_; ++i) {
    point_lights_.push_back(PointLightObject());
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
  plane_.mutable_transform()->SetScale(glm::vec3(10, 1, 10));

  nanosuit_.Init(context, "nanosuit", "nanosuit");
  for (int i = 0; i < nanosuit_.model_part_num(); ++i) {
    glm::quat rotation = glm::angleAxis(-float(M_PI) / 2, glm::vec3(1, 0, 0));
    nanosuit_.mutable_model_part(i)->mutable_transform()->SetRotation(rotation);
    nanosuit_.mutable_model_part(i)->mutable_transform()->SetScale(glm::vec3(0.2, 0.2, 0.2));
    nanosuit_.mutable_model_part(i)->mutable_transform()->SetTranslation(glm::vec3(0, -0.5, 0));
  }
  
  glEnable_(GL_DEPTH_TEST);

  SetupBufferAndPass(context);

  std::vector<glm::vec3> SSAO_noise = util::Noise(4, 4);
  texture_noise_ = context->CreateTexture({4, 4, SSAO_noise, GL_NEAREST, GL_NEAREST});

  samples_ts_ = util::AsArray<64>(util::SampleSemishphere(64));
}

void SSAOScene::OnUpdate(Scene *context)
{
  OnUpdateCommon _(context, "SSAOScene");

  ImGui::Separator();

  plane_.OnUpdate(context);
}

void SSAOScene::SetupBufferAndPass(Scene* context) {
  glm::ivec2 screen_size = context->io().screen_size();
  g_buffer_.Init({screen_size, {renderer::kAttachmentPositionVS, renderer::kAttachmentNormalVS,
                                renderer::kAttachmentColor, renderer::kAttachmentDepth}});
  SSAO_buffer_.Init({screen_size, {renderer::kAttachmentColor, renderer::kAttachmentDepth}});
  blur_buffer_.Init({screen_size, {renderer::kAttachmentColor, renderer::kAttachmentDepth}});
  lighting_buffer_.Init({screen_size, {renderer::kAttachmentColor, renderer::kAttachmentDepth}});

  g_buffer_pass_.Init(&g_buffer_);
  SSAO_pass_.Init(&SSAO_buffer_);
  blur_pass_.Init(&blur_buffer_);
  lighting_pass_.Init(&blur_buffer_, &lighting_buffer_);
}

void SSAOScene::OnRender(Scene *context)
{
  RunGBufferPass(context, &g_buffer_pass_);

  renderer::Texture t = g_buffer_pass_.g_buffer()->GetTexture(renderer::kAttachmentPositionVS.name);
  SSAO_pass_.texture_position_vs = g_buffer_pass_.g_buffer()->GetTexture(renderer::kAttachmentPositionVS.name);
  SSAO_pass_.texture_normal_vs = g_buffer_pass_.g_buffer()->GetTexture(renderer::kAttachmentNormalVS.name);
  SSAO_pass_.texture_noise = texture_noise_;
  SSAO_pass_.samples_ts = samples_ts_;
  RunSSAOPass(context, &SSAO_pass_);

  blur_pass_.texture_SSAO = SSAO_pass_.SSAO_buffer_->GetTexture(renderer::kAttachmentColor.name);
  RunBlurPass(context, &blur_pass_);

  EmptyObject object;
  FullscreenQuadShader full_screen({blur_pass_.texture_blur()}, context, &object);
  object.OnRender(context);

  OnRenderCommon _(context);
}

void SSAOScene::RunGBufferPass(Scene* context, renderer::GBufferPass* g_buffer_pass) {
  g_buffer_pass->Begin();

  SSAOShader(SSAOShader::ParamGBuffer(), context, &plane_);
  plane_.OnRender(context);

  for (int i = 0; i < nanosuit_.model_part_num(); ++i) {
    ModelPartObject* model_part = nanosuit_.mutable_model_part(i);
    SSAOShader(SSAOShader::ParamGBuffer(), context, model_part);
    model_part->OnRender(context);
  }

  g_buffer_pass->End();
}

void SSAOScene::RunSSAOPass(Scene* context, renderer::SSAOPass* SSAO_pass) {
  SSAO_pass->Begin();

  EmptyObject object;
  SSAOShader::ParamSSAO SSAO_param{SSAO_pass->texture_position_vs, SSAO_pass->texture_normal_vs,
                                   SSAO_pass->texture_noise, SSAO_pass->samples_ts};
  SSAOShader(SSAO_param, context, &object);
  object.OnRender(context);

  SSAO_pass->End();
}

void SSAOScene::RunBlurPass(Scene* context, renderer::BlurPass* blue_pass) {
  blur_pass_.Begin();

  EmptyObject object;
  BlurShader({blur_pass_.texture_SSAO, context->io().screen_size()}, context, &object);
  object.OnRender(context);

  blur_pass_.End();
}

void SSAOScene::OnExit(Scene *context)
{
  for (int i = 0; i < point_lights_num_; ++i) {
    point_lights_[i].OnDestory(context);
  }
  coord_.OnDestory(context);
  plane_.OnDestory(context);

  context->SetCamera(nullptr);
}