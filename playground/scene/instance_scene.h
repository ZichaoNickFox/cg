#pragma once

#include "engine/camera.h"
#include "playground/object/model_object.h"
#include "playground/object/sphere_object.h"
#include "playground/scene.h"

class InstanceScene : public Scene {
 public:
  void OnEnter(Context* context) override;
  void OnUpdate(Context* context) override;
  void OnRender(Context* context) override;
  void OnExit(Context* context) override;

 private:
  void InitModelMatrices();

  ModelObject rock_;
  ModelObject planet_;

  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();

  std::vector<glm::mat4> models_;
};

