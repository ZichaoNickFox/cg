#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <unordered_map>

#include "renderer/io.h"
#include "renderer/scene.h"
#include "renderer/util.h"

// CG roadmap? https://github.com/miloyip/game-programmer
class Playground {
 public:
  void Init(const renderer::Scene::Option& option);
  void BeginFrame();
  void Update();
  void Render();
  void EndFrame();
  void Destoy();

  Io* mutable_io() { return current_scene_->mutable_io(); }
  const Io& io() { return current_scene_->io(); }
 
 private:
  std::string current_scene_name_;
  renderer::Scene* current_scene_ = nullptr;

  util::Time frame_start_time_;
};