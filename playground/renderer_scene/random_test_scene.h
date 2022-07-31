#pragma once

#include "renderer/camera.h"
#include "renderer/scene.h"
#include "playground/object/sphere_object.h"

class RandomTestScene : public Scene {
 public:
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  std::shared_ptr<renderer::Camera> camera_ = std::make_shared<renderer::Camera>();
  
  renderer::Texture input_;
  renderer::Texture output_;
};
