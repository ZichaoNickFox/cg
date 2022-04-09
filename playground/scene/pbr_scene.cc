#include "playground/scene/pbr_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "engine/transform.h"
#include "playground/scene/common.h"

void PbrScene::OnEnter(Context *context)
{
  const glm::vec3 kLightColor = glm::vec3(0.0, 1.0, 0.0);
  const glm::vec3 kLightScale = glm::vec3(0.4, 0.4, 0.4);

  std::vector<glm::vec3> light_poses{glm::vec3(2, 2, 0), glm::vec3(2, 0, 2),
                                     glm::vec3(0, 2, 2), glm::vec3(2, 2, 2)};
  for (int i = 0; i < light_num; ++i) {
    lights_.push_back(PointLight());
    lights_[i].mutable_transform()->SetTranslation(light_poses[i]);
    lights_[i].mutable_transform()->SetScale(kLightScale);
    lights_[i].mutable_material()->PushShader(context->GetShader("point_light"));
    lights_[i].mutable_material()->SetVec3("light_color", kLightColor);
  }

  camera_->mutable_transform()->SetTranslation(glm::vec3(-1.0, 1.5, 1.1));
  camera_->SetFarClip(200);
  context->PushCamera(camera_);

  std::vector<glm::vec3> positions{glm::vec3(0, 0, 0), glm::vec3(2, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 2, 0),
                                   glm::vec3(0, 0, 0), glm::vec3(0, 0, 2)};
  std::vector<glm::vec3> colors{glm::vec3(1, 0, 0), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0),
                                glm::vec3(0, 0, 1), glm::vec3(0, 0, 1)};
  coord_.SetData(context, {positions, colors, GL_LINES, 5});

  skybox_.mutable_material()->PushShader(context->GetShader("skybox"));
  skybox_.mutable_material()->SetTexture("texture0", context->GetTexture("skybox"));
  skybox_.mutable_transform()->SetScale(glm::vec3(100, 100, 100));

  sphere_.mutable_material()->PushShader(context->GetShader("phong"));

  plane_.mutable_material()->PushShader(context->GetShader("phong"));

  glEnable(GL_DEPTH_TEST);
}

void PbrScene::OnUpdate(Context *context)
{
  OnUpdateCommon _(context, "PbrScene");

  ImGui::ColorEdit3("light_color", (float*)&light_color_);
  
  for (int i = 0; i < light_num; ++i) {
    lights_[i].OnUpdate(context);
    lights_[i].mutable_material()->SetVec3("light_color", light_color_);
  }

  sphere_.OnUpdate(context);
  coord_.OnUpdate(context);
  skybox_.OnUpdate(context);
  plane_.OnUpdate(context);
}

void PbrScene::OnRender(Context *context)
{
  sphere_.mutable_material()->SetVec3("camera_pos", context->camera().transform().translation());
  for (int i = 0; i < light_num; ++i) {
    lights_[i].OnRender(context);
    // sphere_.mutable_material()->SetVec4();
  }
  sphere_.OnRender(context);
  coord_.OnRender(context);
  skybox_.OnRender(context);
  plane_.OnRender(context);
}

void PbrScene::OnExit(Context *context)
{
  for (int i = 0; i < light_num; ++i) {
    lights_[i].OnDestory(context);
  }
  coord_.OnDestory(context);
  skybox_.OnDestory(context);
  context->PopCamera();
  plane_.OnDestory(context);
}