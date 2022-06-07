#include "playground/scene/common.h"

#include "imgui.h"
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

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
  ImGui::Text("camera_location %s", glm::to_string(context->camera().transform().translation()).c_str());
  ImGui::Text("camera_rotation %s", glm::to_string(context->camera().transform().rotation()).c_str());
  ImGui::Text("camera_front %s", glm::to_string(context->camera().front()).c_str());
  glm::vec3 near_pos_ws, far_pos_ws;
  context->camera().GetPickRay(context->io().normalized_cursor_screen_pos(), &near_pos_ws, &far_pos_ws);
  ImGui::Text("camera_near_pos_ws %s", glm::to_string(near_pos_ws).c_str());
  ImGui::Text("camera_far_pos_ws %s", glm::to_string(far_pos_ws).c_str());
  ImGui::Text("pick dir %s", glm::to_string(glm::normalize(far_pos_ws - near_pos_ws)).c_str());
}

void OnUpdateCommon::InSpectCursor(Context* context) {
  ImGui::Text("cursor pos x %lf", context->io().cursor_screen_pos().x);
  ImGui::Text("cursor pos y %lf", context->io().cursor_screen_pos().y);
  ImGui::Text("cursor screen space x %lf", context->io().normalized_cursor_screen_pos().x);
  ImGui::Text("cursor screen space y %lf", context->io().normalized_cursor_screen_pos().y);
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
    double cursor_delta_x = context->io().GetCursorDelta().x * camera_rotate_speed;
    double cursor_delta_y = context->io().GetCursorDelta().y * camera_rotate_speed;
    camera->RotateHorizontal(cursor_delta_x);
    camera->RotateVerticle(cursor_delta_y);
  }
}

OnRenderCommon::OnRenderCommon(Context* context) {
  DrawWorldCoordAndViewCoord(context);
}

void OnRenderCommon::DrawWorldCoordAndViewCoord(Context* context) {
  glDisable_(GL_DEPTH_TEST);
  enum Type {kWorld = 0, kNum};
  std::array<glm::vec2, kNum> positions_ss = {glm::vec2{0.5f, 0.1f}};
  std::array<glm::vec3, kNum> near_positions_ws;
  std::array<glm::vec3, kNum> far_positions_ws;
  for (int i = kWorld; i < kNum; ++i) {
    context->camera().GetPickRay(positions_ss[i], &near_positions_ws[i], &far_positions_ws[i]);
    glm::vec3 direction = glm::normalize(far_positions_ws[i] - near_positions_ws[i]);
    glm::vec3 world_coord_pos = near_positions_ws[i] + direction * glm::vec3(0.5, 0.5, 0.5);
    Coord coord;
    coord.mutable_transform()->SetTranslation(world_coord_pos);
    coord.mutable_transform()->SetScale(glm::vec3(0.05, 0.05, 0.05));
    LinesShader({0.3}, context, &coord);
    coord.OnRender(context);
  }
  glEnable_(GL_DEPTH_TEST);
}

RaytracingDebugCommon::RaytracingDebugCommon(const engine::Texture& in, Context* context, const LightPath& light_path) {
  EmptyObject empty_object;
  FullscreenQuadShader({in}, context, &empty_object);
  empty_object.OnRender(context);

  glDisable_(GL_DEPTH_TEST);

  std::vector<glm::vec4> colors{{1, 0, 0, 1}, {1, 156 / 255.0, 0, 1}, {1, 1, 0, 1}, {0, 1, 0, 1},
                                {0, 1, 1, 1}, {0, 0, 1, 1}, {1, 0, 1, 1}, {1, 1, 1, 1},
                                {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1},
                                {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1},
                                {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}};
  LinesObject::Mesh mesh{util::AsVector(light_path.light_path), colors, GL_LINE_STRIP};
  LinesObject lines_object;
  lines_object.SetMesh(mesh);
  LinesShader({}, context, &lines_object);
  lines_object.OnRender(context);

  Coord coord;
  LinesShader({}, context, &coord);
  coord.OnRender(context);
  glEnable_(GL_DEPTH_TEST);

}
