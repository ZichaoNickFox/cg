#include "renderer/scene.h"

#include "renderer/debug.h"
#include "renderer/definition.h"
#include "renderer/inspector.h"
#include "renderer/util.h"

namespace renderer {
void Scene::Enter(const std::string& name, Config* config, Io* io, FrameStat* frame_stat) {
  name_ = name;

  config_ = config;
  io_ = io;
  frame_stat_ = frame_stat;

  shader_program_repo_.Init(*config);
  object_repo_.Init(config, &mesh_repo_, &material_repo_, &texture_repo_);

  camera_->SetAspect(float(io_->screen_size().x) / io_->screen_size().y);
  camera_->SetPerspectiveFov(60);

  glEnable_(GL_DEPTH_TEST);

  OnEnter();
}

void Scene::Update() {
  material_repo_.UpdateSSBO();
  light_repo_.UpdateSSBO();
  bvh_.UpdateSSBO();
  primitive_repo_.UpdateSSBO(bvh_.GetPrimitiveSequence());

  MoveCamera();
  OnUpdate();

  inspector_.Inspect(name_, this);
}

void Scene::Render() {
  OnRender();
}

void Scene::Exit() {
  OnExit();
}

const Camera& Scene::camera() const {
  CGCHECK(camera_) << "camera must be nullptr";
  return *camera_.get();
}

Camera* Scene::mutable_camera() {
  return camera_.get();
}

void Scene::MoveCamera() {
  float camera_move_speed = camera_->move_speed() / 200.0;
  float camera_rotate_speed = camera_->rotate_speed() / 3000.0;

  if (io_->gui_captured_cursor()) {
    return;
  }
  if (io_->HadKeyInput("w")) {
    camera_->MoveForwardWS(camera_move_speed);
  } else if (io_->HadKeyInput("s")) {
    camera_->MoveForwardWS(-camera_move_speed);
  } else if (io_->HadKeyInput("a")) {
    camera_->MoveRightWS(-camera_move_speed);
  } else if (io_->HadKeyInput("d")) {
    camera_->MoveRightWS(camera_move_speed);
  } else if (io_->HadKeyInput("esc")) {
    exit(0);
  }
  
  if (io_->left_button_pressed()) {
    double cursor_delta_x = io_->GetCursorDelta().x * camera_rotate_speed;
    double cursor_delta_y = io_->GetCursorDelta().y * camera_rotate_speed;
    camera_->RotateHorizontal(cursor_delta_x);
    camera_->RotateVerticle(cursor_delta_y);
  }
}
} // namespace scene
