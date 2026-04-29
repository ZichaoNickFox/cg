#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <string>
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
  void SetPresentationRuntimeName(std::string runtime_name);

  cg::Io* mutable_io() { return &io_; }
  const cg::Io& io() { return io_; }

 private:
  void EnsureScene();
  void SwitchToScene(const std::string& scene_id);
  void DrawSceneSelector();

  cg::Config config_;
  cg::Io io_;
  cg::FrameStat frame_stat_;

  std::string current_scene_name_;
  std::string pending_scene_name_;
  bool force_reload_scene_ = false;
  bool scene_selector_popup_requested_ = false;
  bool show_unsupported_scenes_ = false;
  int selected_scene_category_index_ = 0;
  std::string presentation_runtime_name_;
  std::unique_ptr<cg::Scene> current_scene_;

  util::Time frame_start_time_;
};
