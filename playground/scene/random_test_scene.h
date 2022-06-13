#pragma once

#include "engine/camera.h"
#include "playground/object/sphere_object.h"
#include "playground/scene.h"

class RandomTestScene : public Scene {
 public:
  void OnEnter(Context* context) override;
  void OnUpdate(Context* context) override;
  void OnRender(Context* context) override;
  void OnExit(Context* context) override;

 private:
  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();
  
  engine::Texture input_;
  engine::Texture output_;
};
