#include "renderer/scene.h"

#include "base/debug.h"
#include "base/util.h"
#include "renderer/definition.h"
#include "renderer/inspector.h"
#include "rhi/device.h"

namespace cg {
namespace {
float CameraAspectFromIo(const Io& io) {
  const glm::ivec2 size = io.framebuffer_size().x > 0 && io.framebuffer_size().y > 0
                              ? io.framebuffer_size()
                              : io.screen_size();
  CGCHECK(size.x > 0 && size.y > 0) << "Invalid viewport size: " << size.x << "x" << size.y;
  return static_cast<float>(size.x) / static_cast<float>(size.y);
}
}  // namespace

void Scene::Enter(const std::string& name, Config* config, Io* io, FrameStat* frame_stat) {
  name_ = name;

  config_ = config;
  io_ = io;
  frame_stat_ = frame_stat;

  shader_program_repo_.Init(*config);
  texture_repo_.Init(*config);
  object_repo_.Init(config, &mesh_repo_, &material_repo_, &texture_repo_);

  camera_->SetAspect(CameraAspectFromIo(*io_));
  camera_->SetPerspectiveFov(60);

  rhi::GetDevice().ApplyRenderState({
      .depth_test_enabled = true,
  });

  OnEnter();

  light_repo_.Add(primitive_repo_, material_repo_);
}

void Scene::Update() {
  camera_->SetAspect(CameraAspectFromIo(*io_));

  const rhi::Capabilities& gpu_caps = rhi::GetCapabilities();
  if (gpu_caps.supports_storage_buffers) {
    material_repo_.UpdateSSBO();
    bvh_.UpdateSSBO();
    primitive_repo_.UpdateSSBO(bvh_.GetPrimitiveSequence());
    light_repo_.UpdateSSBO();
  } else {
    static bool warned = false;
    if (!warned) {
      CGLOG(ERROR) << "GPU storage buffers are unavailable on this machine (API="
                   << rhi::GetSceneApiName()
                   << "). GPU scenes that rely on storage buffers / compute shaders are disabled.";
      warned = true;
    }
  }

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

const Texture& Scene::GetTexture(const std::string& name) const {
  return texture_repo_.GetTexture(name);
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
