#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

#include "playground/context.h"
#include "playground/scene.h"

class Framework {
 public:
  void Init(const std::string& config_path);
  void Update();
  void Gui();
  void Render();

  void SwitchScene(SceneType scene_type, bool ignore_current_scene = false);
 
 private:
  void InitScene();
  void InitContext(const std::string& config_path);

  std::unordered_map<SceneType, std::unique_ptr<Scene>> scene_map_;

  Context context_;
};