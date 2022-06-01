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
#include "playground/object/sphere.h"
#include "playground/scene/common.h"

void SSAOScene::OnEnter(Context *context)
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

  std::vector<glm::vec3> SSAO_noise = engine::Noise(4, 4);
  texture_noise_ = context->CreateTexture({4, 4, SSAO_noise, GL_NEAREST, GL_NEAREST});

  samples_ts_ = util::AsArray<64>(engine::SampleSemishphere(64));
}

void SSAOScene::OnUpdate(Context *context)
{
  OnUpdateCommon _(context, "SSAOScene");

  ImGui::Separator();

  plane_.OnUpdate(context);
}

void SSAOScene::SetupBufferAndPass(Context* context) {
  glm::ivec2 screen_size = context->io().screen_size();
  g_buffer_.Init({screen_size, {engine::kAttachmentPositionVS, engine::kAttachmentNormalVS,
                                engine::kAttachmentColor, engine::kAttachmentDepth}});
  SSAO_buffer_.Init({screen_size, {engine::kAttachmentColor, engine::kAttachmentDepth}});
  blur_buffer_.Init({screen_size, {engine::kAttachmentColor, engine::kAttachmentDepth}});
  lighting_buffer_.Init({screen_size, {engine::kAttachmentColor, engine::kAttachmentDepth}});

  g_buffer_pass_.Init(&g_buffer_);
  SSAO_pass_.Init(&SSAO_buffer_);
  blur_pass_.Init(&blur_buffer_);
  lighting_pass_.Init(&blur_buffer_, &lighting_buffer_);
}

void SSAOScene::OnRender(Context *context)
{
  RunGBufferPass(context, &g_buffer_pass_);

  engine::Texture t = g_buffer_pass_.g_buffer()->GetTexture(engine::kAttachmentPositionVS.name);
  SSAO_pass_.texture_position_vs = g_buffer_pass_.g_buffer()->GetTexture(engine::kAttachmentPositionVS.name);
  SSAO_pass_.texture_normal_vs = g_buffer_pass_.g_buffer()->GetTexture(engine::kAttachmentNormalVS.name);
  SSAO_pass_.texture_noise = texture_noise_;
  SSAO_pass_.samples_ts = samples_ts_;
  RunSSAOPass(context, &SSAO_pass_);

  blur_pass_.texture_SSAO = SSAO_pass_.SSAO_buffer_->GetTexture(engine::kAttachmentColor.name);
  RunBlurPass(context, &blur_pass_);

  EmptyObject object;
  FullscreenQuadShader full_screen({blur_pass_.texture_blur()}, context, &object);
  object.OnRender(context);

  OnRenderCommon _(context);
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
  SSAO_pass->Begin();

  EmptyObject object;
  SSAOShader::ParamSSAO SSAO_param{SSAO_pass->texture_position_vs, SSAO_pass->texture_normal_vs,
                                   SSAO_pass->texture_noise, SSAO_pass->samples_ts};
  SSAOShader(SSAO_param, context, &object);
  object.OnRender(context);

  SSAO_pass->End();
}

void SSAOScene::RunBlurPass(Context* context, engine::BlurPass* blue_pass) {
  blur_pass_.Begin();

  EmptyObject object;
  BlurShader({blur_pass_.texture_SSAO, context->io().screen_size()}, context, &object);
  object.OnRender(context);

  blur_pass_.End();
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