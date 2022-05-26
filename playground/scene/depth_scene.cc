#include "playground/scene/depth_scene.h"

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

void DepthScene::OnEnter(Context *context)
{
  for (int i = 0; i < point_lights_num_; ++i) {
    point_lights_.push_back(PointLight());
    glm::vec3 point_light_pos(engine::RandFromTo(-5, 5), engine::RandFromTo(0, 5), engine::RandFromTo(-5, 5));
    point_lights_[i].mutable_transform()->SetTranslation(point_light_pos);
    point_lights_[i].mutable_transform()->SetScale(glm::vec3(0.2, 0.2, 0.2));
    glm::vec4 color(engine::RandFromTo(0, 1), engine::RandFromTo(0, 1), engine::RandFromTo(0, 1), 1.0);
    point_lights_[i].SetColor(color);
  }

  camera_->mutable_transform()->SetTranslation(glm::vec3(6.85, 3.74, -3.47));
  camera_->mutable_transform()->SetRotation(glm::quat(-0.56, 0.17, -0.77, -0.21));
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

  depth_framebuffer_.Init({context->framebuffer_size(), {engine::kAttachmentColor, engine::kAttachmentDepth}});
  depth_buffer_pass_.Init(&depth_framebuffer_, camera_);
}

void DepthScene::OnUpdate(Context *context)
{
  OnUpdateCommon _(context, "DepthScene");

  ImGui::Separator();

  ImGui::Text("Camera Type");
  ImGui::SameLine();

  for (int i = 0; i < point_lights_num_; ++i) {
    point_lights_[i].OnUpdate(context);
  }

  coord_.OnUpdate(context);
  plane_.OnUpdate(context);
}

void DepthScene::OnRender(Context *context)
{
  RunDepthBufferPass(context, &depth_buffer_pass_);

  EmptyObject quad;
  FullscreenQuadShader({depth_buffer_pass_.GetTexture(engine::kAttachmentColor.name)}, context, &quad);
  quad.OnRender(context);
}

void DepthScene::RunDepthBufferPass(Context* context, engine::DepthBufferPass* depth_buffer_pass) {
  depth_buffer_pass->Begin();

  DepthBufferShader::Param param{camera_, context->GetShader("depth_buffer")};
  DepthBufferShader{param, &plane_};
  plane_.OnRender(context);

  for (int i = 0; i < nanosuit_.model_part_num(); ++i) {
    DepthBufferShader{param, nanosuit_.mutable_model_part(i)};
    nanosuit_.mutable_model_part(i)->OnRender(context);
  }

  depth_buffer_pass->End();
}

void DepthScene::OnExit(Context *context)
{
  for (int i = 0; i < point_lights_num_; ++i) {
    point_lights_[i].OnDestory(context);
  }
  coord_.OnDestory(context);
  plane_.OnDestory(context);

  context->SetCamera(nullptr);
}
