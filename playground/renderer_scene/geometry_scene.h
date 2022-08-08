#pragma once

#include <vector>

#include "base/geometry.h"
#include "renderer/camera.h"
#include "renderer/scene.h"
#include "renderer/ssbo.h"

class GeometryScene : public cg::Scene {
 public:
  GeometryScene() : ssbo_triangle_(SSBO_USER_0), ssbo_aabb_(SSBO_USER_1) {}
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  static constexpr int kTriangleNum = 100;
  std::array<cg::Triangle, 100> triangles_;

  static constexpr int kAABBNum = 100;
  std::array<cg::AABB, 100> aabbs_;

  void Rasterization();
  void Raytracing();

  cg::SSBO ssbo_triangle_;
  cg::SSBO ssbo_aabb_;

  cg::Texture canvas_;
};

