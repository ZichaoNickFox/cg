#pragma once

#include "renderer/camera.h"
#include "renderer/geometry.h"
#include "renderer/scene.h"

class GeometryScene : public Scene {
 public:
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  static constexpr int kTriangleNum = 100;
  std::array<renderer::Triangle, kTriangleNum> triangles_;

  static constexpr int kAABBNum = 100;
  std::array<renderer::AABB, kAABBNum> aabbs_;

  std::shared_ptr<renderer::Camera> camera_ = std::make_shared<renderer::Camera>();
};

