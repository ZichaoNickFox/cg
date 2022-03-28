#pragma once

#include <stack>

#include <glog/logging.h>

#include "engine/camera.h"
#include "playground/config.pb.h"
#include "playground/io.h"
#include "playground/shader_repo.h"
#include "playground/texture_repo.h"

class Context {
 public:
  void Init(const Config& config);

  void SetCurrentScene(const std::string& current_scene) { current_scene_ = current_scene; }
  const std::string& current_scene() const { return current_scene_; }

  void SetNextScene(const std::string& next_scene) { next_scene_ = next_scene; }
  const std::string& next_scene() const { return next_scene_; }

  const ShaderRepo& shader_repo() { return shader_repo_; }
  const TextureRepo& texture_repo() { return texture_repo_; }
  ShaderRepo* mutable_shader_repo() { return &shader_repo_; }
  TextureRepo* mutable_texture_repo() { return &texture_repo_; }

  const Io& io() { return io_; }
  Io* mutable_io() { return &io_; }

  void PushCamera(std::shared_ptr<engine::Camera> camera) { cameras_.push(camera); }
  void PopCamera() { cameras_.pop(); }
  const engine::Camera& camera();
  engine::Camera* mutable_camera();

  void SetFrameInternal(int frame_interval);
  int frame_interval() { return frame_interval_; }
  int fps() { return fps_; }

 private:
  std::string current_scene_;
  std::string next_scene_;
  
  ShaderRepo shader_repo_;
  TextureRepo texture_repo_;

  Io io_;

  std::stack<std::weak_ptr<engine::Camera>> cameras_;

  int frame_interval_;
  int fps_;
};