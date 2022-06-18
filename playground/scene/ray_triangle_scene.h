#pragma once

#include "engine/camera.h"
#include "engine/geometry.h"
#include "playground/scene.h"

class RayTriangleScene : public Scene {
 public:
  void OnEnter(Context* context) override;
  void OnUpdate(Context* context) override;
  void OnRender(Context* context) override;
  void OnExit(Context* context) override;

 private:
  static constexpr int kTriangleNum = 1000;
  std::array<engine::Triangle, kTriangleNum> triangles_;

  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();
};

