#pragma once

#include <stack>

#include <glog/logging.h>

#include "engine/camera.h"
#include "engine/config.pb.h"
#include "engine/io.h"
#include "engine/shader_repo.h"
#include "engine/texture_repo.h"
#include "playground/scene.pb.h"

class Context {
 public:
  void Init(const Config& config);

  void SetCurrentSceneType(SceneType current_scene_type) { current_scene_type_ = current_scene_type; }
  SceneType current_scene_type() const { return current_scene_type_; }

  void SetNextSceneType(SceneType next_scene_type) { next_scene_type_ = next_scene_type; }
  SceneType next_scene_type() const { return next_scene_type_; }

  const engine::ShaderRepo& shader_repo() { return shader_repo_; }
  const engine::TextureRepo& texture_repo() { return texture_repo_; }
  engine::ShaderRepo* mutable_shader_repo() { return &shader_repo_; }
  engine::TextureRepo* mutable_texture_repo() { return &texture_repo_; }

  const engine::Io& io() { return io_; }
  engine::Io* mutable_io() { return &io_; }

  void PushCamera(std::shared_ptr<engine::Camera> camera) { cameras_.push(camera); }
  void PopCamera() { cameras_.pop(); }
  const engine::Camera& camera();
  engine::Camera* mutable_camera();

 private:
  SceneType current_scene_type_;
  SceneType next_scene_type_;
  
  engine::ShaderRepo shader_repo_;
  engine::TextureRepo texture_repo_;

  engine::Io io_;

  std::stack<std::weak_ptr<engine::Camera>> cameras_;
};