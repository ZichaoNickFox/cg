#include "playground/scene/common.h"

#include "imgui.h"
#include <glm/gtx/string_cast.hpp>
#include <glog/logging.h>

OnUpdateCommon::OnUpdateCommon(Context* context, const std::string& title) {
  bool open = true;
  ImGui::Begin(title.c_str(), &open, ImGuiWindowFlags_AlwaysAutoResize);
  GuiFps(context);
  ImGui::Separator();

  MoveCamera(context);
  InspectCamera(context);
  ImGui::Separator();

  InSpectCursor(context);
  ImGui::Separator();
}

OnUpdateCommon::~OnUpdateCommon() {
  ImGui::End();
}

void OnUpdateCommon::InspectCamera(Context* context) {
  ImGui::Text("camera_location %s", glm::to_string(context->camera().transform().translation()).c_str());
  ImGui::Text("camera_rotation %s", glm::to_string(context->camera().transform().rotation()).c_str());
  ImGui::Text("camera_front %s", glm::to_string(context->camera().front()).c_str());
}

void OnUpdateCommon::InSpectCursor(Context* context) {
  ImGui::Text("cursor pos x %lf", context->io().cursor_screen_pos_x());
  ImGui::Text("cursor pos y %lf", context->io().cursor_screen_pos_y());
  ImGui::Text("cursor screen space x %lf", context->io().cursor_screen_pos_x() / context->io().screen_size().x);
  ImGui::Text("cursor screen space y %lf", context->io().cursor_screen_pos_y() / context->io().screen_size().y);
}

void OnUpdateCommon::GuiFps(Context* context) {
  ImGui::Text("frame interval : ");
  ImGui::SameLine();
  ImGui::Text("%d", context->frame_interval());
  ImGui::Text("FPS : ");
  ImGui::SameLine();
  ImGui::Text("%d", context->fps());
}

void OnUpdateCommon::MoveCamera(Context* context) {
  ImGui::SliderFloat("camera move speed", context->mutable_camera_move_speed(), 1, 10);
  ImGui::SliderFloat("rotate speed", context->mutable_camera_rotate_speed(), 1, 10);
  float camera_move_speed = context->camera_move_speed() / 200.0;
  float camera_rotate_speed = context->camera_rotate_speed() / 3000.0;

  if (context->io().gui_captured_cursor()) {
    return;
  }
  engine::Camera* camera = context->mutable_camera();
  if (context->io().HadKeyInput("w")) {
    camera->MoveForward(camera_move_speed);
  } else if (context->io().HadKeyInput("s")) {
    camera->MoveForward(-camera_move_speed);
  } else if (context->io().HadKeyInput("a")) {
    camera->MoveRight(-camera_move_speed);
  } else if (context->io().HadKeyInput("d")) {
    camera->MoveRight(camera_move_speed);
  } else if (context->io().HadKeyInput("esc")) {
    exit(0);
  }
  
  if (context->io().left_button_pressed()) {
    double cursor_delta_x = context->io().GetCursorDeltaX() * camera_rotate_speed;
    double cursor_delta_y = context->io().GetCursorDeltaY() * camera_rotate_speed;
    camera->RotateHorizontal(cursor_delta_x);
    camera->RotateVerticle(cursor_delta_y);
  }
}