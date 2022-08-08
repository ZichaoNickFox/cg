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

  cg::Io* mutable_io() { return &io_; }
  const cg::Io& io() { return io_; }
 
 private:
  cg::Config config_;
  cg::Io io_;
  cg::FrameStat frame_stat_;

  std::string current_scene_name_;
  cg::Scene* current_scene_ = nullptr;

  util::Time frame_start_time_;
};