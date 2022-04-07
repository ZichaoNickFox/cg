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
  const glm::vec3 kLightPos = glm::vec3(1, 1, 0);
  const glm::vec3 kLightScale = glm::vec3(0.4, 0.4, 0.4);
  engine::Transform light_transform(kLightPos, glm::quat(glm::vec3(0, 0, 0)), kLightScale);
  light_.SetTransform(light_transform);
  light_.mutable_material()->PushShader(context->GetShader("point_light"));
  light_.mutable_material()->SetVec3("light_color", kLightColor);

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

  sphere_.mutable_material()->PushShader(context->GetShader("texture0"));
  sphere_.mutable_material()->SetTexture("texture0", context->GetTexture("opengl0"));

  glEnable(GL_DEPTH_TEST);
}

void PbrScene::OnUpdate(Context *context)
{
  ControlCameraByIo(context);
  light_.OnUpdate(context);
  light_.mutable_transform()->SetScale(light_scale_);
  light_.mutable_material()->SetVec3("light_color", light_color_);

  sphere_.OnUpdate(context);
  coord_.OnUpdate(context);
  skybox_.OnUpdate(context);
}

void PbrScene::OnGui(Context *context)
{
  bool open = true;
  ImGui::Begin("SkyScene", &open, ImGuiWindowFlags_AlwaysAutoResize);
  RenderFps(context);

  ImGui::Separator();

  ImGui::SliderFloat3("light_scale", (float*)&light_scale_, 0, 2);
  ImGui::ColorEdit3("light_color", (float*)&light_color_);

  ImGui::Separator();

  ImGui::Text("camera_location %s", glm::to_string(context->camera().transform().translation()).c_str());
  ImGui::Text("camera_front %s", glm::to_string(context->camera().front()).c_str());

  ImGui::End();
}

void PbrScene::OnRender(Context *context)
{
  sphere_.OnRender(context);
  light_.OnRender(context);
  coord_.OnRender(context);
  skybox_.OnRender(context);
}

void PbrScene::OnExit(Context *context)
{
  light_.OnDestory(context);
  coord_.OnDestory(context);
  skybox_.OnDestory(context);
  context->PopCamera();
}