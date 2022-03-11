#include "playground/scene/common.h"

#include "imgui.h"

void RenderGoToGallery(Context* context) {
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::Begin("GoToGalleryWidown");
  if (ImGui::Button("Go To Gallery")) {
    context->SetNextSceneType(SceneType::Gallery);
  }
  ImGui::End();
}