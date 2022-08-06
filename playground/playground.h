#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <unordered_map>

#include "renderer/config.h"
#include "renderer/io.h"
#include "renderer/scene.h"
#include "base/util.h"

// CG roadmap? https://github.com/miloyip/game-programmer
class Playground {
 public:
  Playground();
  void BeginFrame();
  void Update();
  void Render();
  void EndFrame();
  void Destoy();

  renderer::Io* mutable_io() { return &io_; }
  const renderer::Io& io() { return io_; }
 
 private:
  renderer::Config config_;
  renderer::Io io_;
  renderer::FrameStat frame_stat_;

  std::string current_scene_name_;
  renderer::Scene* current_scene_ = nullptr;

  util::Time frame_start_time_;
};