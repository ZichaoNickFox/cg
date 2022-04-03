#include "playground/scene/model_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>
#include <vector>

#include "playground/model_repo.h"
#include "playground/scene/common.h"
#include "playground/util.h"

void ModelScene::OnEnter(Context *context)
{
  glEnable(GL_DEPTH_TEST);

  camera_->mutable_transform()->SetTranslation(glm::vec3(0, 10, 20));
  context->PushCamera(camera_);

  nanosuit_.Init(context, "nanosuit1", "nanosuit");
  for (int i = 0; i < nanosuit_.model_part_num(); ++i) {
    nanosuit_.mutable_materials(i)->PushShader(context->GetShader("nanosuit"));
  }
}

void ModelScene::OnUpdate(Context *context)
{
  ControlCameraByIo(context);
}

void ModelScene::OnGui(Context *context)
{
  bool open = true;
  ImGui::Begin("Model Scene", &open, ImGuiWindowFlags_AlwaysAutoResize);
  RenderFps(context);
  nanosuit_.ModelInspector();
  ImGui::End();
}

void ModelScene::OnRender(Context *context)
{
  nanosuit_.OnRender(context);
}

void ModelScene::OnExit(Context *context)
{
  context->PopCamera();
}