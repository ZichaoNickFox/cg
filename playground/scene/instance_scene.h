#pragma once

#include "renderer/camera.h"
#include "renderer/scene.h"
#include "playground/object/sphere_object.h"

class InstanceScene : public Scene {
 public:
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  void InitModelMatrices();

  ModelObject rock_;
  ModelObject planet_;

  std::shared_ptr<renderer::Camera> camera_ = std::make_shared<renderer::Camera>();

  std::vector<glm::mat4> models_;
};

