#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

#include "engine/io.h"
#include "playground/context.h"
#include "playground/scene.h"

class Framework {
 public:
  void Init(const std::string& config_path);
  void BeginFrame();
  void Update();
  void Gui();
  void Render();
  void EndFrame();

  void SwitchScene(SceneType scene_type, bool ignore_current_scene = false);
  engine::Io* mutable_io() { return context_.mutable_io(); }
  const engine::Io& io() { return context_.io(); }
 
 private:
  void InitScene();
  void InitContext(const std::string& config_path);

  std::unordered_map<SceneType, std::unique_ptr<Scene>> scene_map_;

  Context context_;
};