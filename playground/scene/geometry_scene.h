#pragma once

#include "renderer/camera.h"
#include "renderer/geometry.h"
#include "renderer/scene.h"
#include "renderer/ssbo.h"

class GeometryScene : public renderer::Scene {
 public:
  GeometryScene() : ssbo_triangle_(SSBO_USER_0), ssbo_aabb_(SSBO_USER_1) {}
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  static constexpr int kTriangleNum = 100;
  std::array<renderer::Triangle, kTriangleNum> triangles_;

  static constexpr int kAABBNum = 100;
  std::array<renderer::AABB, kAABBNum> aabbs_;

  void Rasterization();
  void Raytracing();

  renderer::SSBO ssbo_triangle_;
  renderer::SSBO ssbo_aabb_;

  renderer::Texture canvas_;
};

