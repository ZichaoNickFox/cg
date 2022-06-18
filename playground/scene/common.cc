#include "playground/scene/common.h"

#include "imgui.h"
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "engine/color.h"
#include "engine/debug.h"
#include "engine/io.h"
#include "playground/object/lines_object.h"
#include "playground/shaders.h"

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

  ReloadShaders(context);

  ImGui::Separator();
}

OnUpdateCommon::~OnUpdateCommon() {
  ImGui::End();
}

void OnUpdateCommon::InspectCamera(Context* context) {
  std::string camera_pos_ws = glm::to_string(context->camera().transform().translation());
  ImGui::Text("camera_pos_ws %s", camera_pos_ws.c_str());
  ImGui::SameLine();
  if (ImGui::Button("Copy##camera_pos_ws")) {
    context->set_clipboard_string_func()(camera_pos_ws.c_str());
  }

  std::string camera_dir_ws = glm::to_string(context->camera().front_ws());
  ImGui::Text("camera_dir_ws %s", camera_dir_ws.c_str());
  ImGui::SameLine();
  if (ImGui::Button("Copy##camera_dir_ws")) {
    context->set_clipboard_string_func()(camera_dir_ws.c_str());
  }

  std::string camera_rotation_quat = glm::to_string(context->camera().transform().rotation());
  ImGui::Text("camera_rotation %s", camera_rotation_quat.c_str());
  ImGui::SameLine();
  if (ImGui::Button("Copy##camera_roatation_quat")) {
    context->set_clipboard_string_func()(camera_rotation_quat.c_str());
  }

  glm::vec3 near_pos_ws, far_pos_ws;
  context->camera().GetPickRay(context->io().GetCursorPosSS(), &near_pos_ws, &far_pos_ws);
  ImGui::Text("camera_near_pos_ws %s", glm::to_string(near_pos_ws).c_str());
  ImGui::Text("camera_far_pos_ws %s", glm::to_string(far_pos_ws).c_str());
  ImGui::Text("pick dir %s", glm::to_string(glm::normalize(far_pos_ws - near_pos_ws)).c_str());
}

void OnUpdateCommon::InSpectCursor(Context* context) {
  ImGui::Text("cursor pos x %lf", context->io().GetCursorWindowPos().x);
  ImGui::Text("cursor pos y %lf", context->io().GetCursorWindowPos().y);
  ImGui::Text("cursor screen space x %lf", context->io().GetCursorPosSS().x);
  ImGui::Text("cursor screen space y %lf", context->io().GetCursorPosSS().y);
}

void OnUpdateCommon::ReloadShaders(Context* context) {
  if (ImGui::Button("Reload Shaders")) {
    context->mutable_shader_repo()->ReloadShaders();
  }
}

void OnUpdateCommon::GuiFps(Context* context) {
  context->frame_stat().Gui();
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
    camera->MoveForwardWS(camera_move_speed);
  } else if (context->io().HadKeyInput("s")) {
    camera->MoveForwardWS(-camera_move_speed);
  } else if (context->io().HadKeyInput("a")) {
    camera->MoveRightWS(-camera_move_speed);
  } else if (context->io().HadKeyInput("d")) {
    camera->MoveRightWS(camera_move_speed);
  } else if (context->io().HadKeyInput("esc")) {
    exit(0);
  }
  
  if (context->io().left_button_pressed()) {
    double cursor_delta_x = context->io().GetCursorDelta().x * camera_rotate_speed;
    double cursor_delta_y = context->io().GetCursorDelta().y * camera_rotate_speed;
    camera->RotateHorizontal(cursor_delta_x);
    camera->RotateVerticle(cursor_delta_y);
  }
}

OnRenderCommon::OnRenderCommon(Context* context) {
  DrawViewCoord(context);
}

void OnRenderCommon::DrawViewCoord(Context* context) {
  glDisable_(GL_DEPTH_TEST);
  glm::vec3 near_pos_ws, far_pos_ws;
  context->camera().GetPickRay(glm::vec2(0.5, 0.1), &near_pos_ws, &far_pos_ws);
  glm::vec3 direction = glm::normalize(far_pos_ws - near_pos_ws);
  glm::vec3 world_coord_pos = near_pos_ws + direction * glm::vec3(0.5, 0.5, 0.5);
  CoordObject coord;
  coord.mutable_transform()->SetTranslation(world_coord_pos);
  coord.mutable_transform()->SetScale(glm::vec3(0.05, 0.05, 0.05));
  LinesShader({0.3}, context, &coord);
  coord.OnRender(context);
  glEnable_(GL_DEPTH_TEST);
}

RaytracingDebugCommon::LightPath::LightPath() {
  for (int i = 0; i < 20; ++i) {
    light_path[i] = glm::vec4(9999, 9999, 9999, 1);
  }
}

RaytracingDebugCommon::RaytracingDebugCommon(const engine::Texture& in, Context* context, const LightPath& light_path) {
  EmptyObject empty_object;
  FullscreenQuadShader({in}, context, &empty_object);
  empty_object.OnRender(context);

  glDisable_(GL_DEPTH_TEST);

  std::vector<glm::vec4> colors{engine::kRed, engine::kOrange, engine::kYellow, engine::kGreen,
                                engine::kCyan, engine::kBlue, engine::kPurple, engine::kWhite,
                                engine::kWhite, engine::kWhite, engine::kWhite, engine::kWhite,
                                engine::kWhite, engine::kWhite, engine::kWhite, engine::kWhite,
                                engine::kWhite, engine::kWhite, engine::kWhite, engine::kWhite};
  LinesObject::Mesh mesh{util::AsVector(light_path.light_path), colors, GL_LINE_STRIP};
  LinesObject lines_object;
  lines_object.SetMesh(mesh);
  LinesShader({}, context, &lines_object);
  lines_object.OnRender(context);

  CoordObject coord;
  LinesShader({}, context, &coord);
  coord.OnRender(context);
  glEnable_(GL_DEPTH_TEST);

}
