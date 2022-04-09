#pragma once

#include "engine/camera.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "engine/transform.h"
#include "playground/context.h"
#include "playground/object/cube.h"
#include "playground/object/model.h"
#include "playground/scene.h"

class ModelScene : public Scene {
 public:
  void OnEnter(Context* context);
  void OnUpdate(Context* context);
  void OnRender(Context* contexnt);
  void OnExit(Context* context);

 private:
  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();
  Model nanosuit_;
};