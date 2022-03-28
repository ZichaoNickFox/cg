#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <unordered_map>

#include "playground/io.h"
#include "playground/context.h"
#include "playground/scene.h"
#include "playground/util.h"

class Playground {
 public:
  void Init(const std::string& config_path);
  void BeginFrame();
  void Update();
  void Gui();
  void Render();
  void EndFrame();

  void SwitchScene(const std::string& scene, bool ignore_current_scene = false);
  Io* mutable_io() { return context_.mutable_io(); }
  const Io& io() { return context_.io(); }
 
 private:
  void InitScene();
  void InitContext(const std::string& config_path);

  std::unordered_map<std::string, std::unique_ptr<Scene>> scene_map_;

  Context context_;

  util::Time frame_start_time_;
};