#include "playground/scene/common.h"

#include "imgui.h"
#include <glog/logging.h>

OnUpdateCommon::OnUpdateCommon(Context* context, const std::string& title) {
  ControlCameraByIo(context);
  bool open = true;
  ImGui::Begin(title.c_str(), &open, ImGuiWindowFlags_AlwaysAutoResize);
  RenderFps(context);
  ImGui::Separator();
}

OnUpdateCommon::~OnUpdateCommon() {
  ImGui::End();
}

void OnUpdateCommon::RenderFps(Context* context) {
  ImGui::Text("frame interval : ");
  ImGui::SameLine();
  ImGui::Text("%d", context->frame_interval());
  ImGui::Text("FPS : ");
  ImGui::SameLine();
  ImGui::Text("%d", context->fps());
}

void OnUpdateCommon::ControlCameraByIo(Context* context) {
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