#pragma once

#include "engine/camera.h"
#include "playground/object/sphere.h"
#include "playground/scene.h"

class ComputeShaderScene : public Scene {
 public:
  void OnEnter(Context* context) override;
  void OnUpdate(Context* context) override;
  void OnRender(Context* context) override;
  void OnExit(Context* context) override;

 private:
  std::vector<glm::vec3> canvas_;
  engine::Texture texture_canvas_;
};
