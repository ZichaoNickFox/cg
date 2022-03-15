#include "playground/scene/common.h"

#include "imgui.h"
#include <glog/logging.h>

void RenderGoToGallery(Context* context) {
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  bool open = true;
  ImGui::Begin("GoToGalleryWidown", &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
  ImGui::Text("Scene : %s", SceneType_Name(context->current_scene_type()).c_str());
  if (ImGui::Button("Go To Gallery")) {
    context->SetNextSceneType(SceneType::Gallery);
  }
  ImGui::End();
}

void ControlCameraByIo(Context* context) {
  if (context->io().gui_captured_mouse()) {
    return;
  }
  engine::Camera* camera = context->mutable_camera();
  const float kMoveSpeed = 0.1;
  if (context->io().HadKeyInput("w")) {
    camera->MoveForward(kMoveSpeed);
  } else if (context->io().HadKeyInput("s")) {
    camera->MoveForward(-kMoveSpeed);
  } else if (context->io().HadKeyInput("a")) {
    camera->MoveRight(-kMoveSpeed);
  } else if (context->io().HadKeyInput("d")) {
    camera->MoveRight(kMoveSpeed);
  }

  if (context->io().left_button_pressed()) {
    const float kRotateSpeedFator = 0.001;
    double cursor_delta_x = context->io().GetCursorDeltaX() * kRotateSpeedFator;
    double cursor_delta_y = context->io().GetCursorDeltaY() * kRotateSpeedFator;
    camera->RotateHorizontal(cursor_delta_x);
    camera->RotateVerticle(cursor_delta_y);
  }
}