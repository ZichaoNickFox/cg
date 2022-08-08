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
  std::shared_ptr<cg::Camera> camera_ = std::make_shared<cg::Camera>();
  
  cg::Texture input_;
  cg::Texture output_;
};
