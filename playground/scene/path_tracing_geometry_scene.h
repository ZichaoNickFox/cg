#pragma once

#include <unordered_map>

#include "renderer/automic_counter.h"
#include "renderer/camera.h"
#include "renderer/definition.h"
#include "renderer/geometry.h"
#include "renderer/scene.h"
#include "renderer/ssbo.h"

class PathTracingGeometryScene : public renderer::Scene {
 public:
  PathTracingGeometryScene() {}
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  std::unordered_map<std::string, renderer::Sphere> sphere_map_ = {
    {"light", {1, {0, 5.4, 1}, {1, 1, 1, 1}, 3}},
    {"ground", {2, {0, -100.5, 1}, {0.8, 0.8, 0.8, 1.0}, 100}},
    {"ceiling", {3, {0, 102.5, 1}, {0.8, 0.8, 0.8, 1.0}, 100}},
    {"back_wall", {4, {0, 1, -101}, {0.8, 0.8, 0.8, 1.0}, 100}},
    {"left_wall", {5, {-101.5, 0, 1}, {0.0, 0.6, 0.0, 1.0}, 100}},
    {"right_wall", {6, {101.5, 0, 1}, {0.6, 0.0, 0.0, 1.0}, 100}},
    {"diffuse_ball", {7, {0, -0.2, 1.5}, {0.8, 0.3, 0.3, 1.0}, 0.3}},
    {"metal_ball", {8, {0.8, 0.2, 1}, {0.6, 0.8, 0.8, 1.0}, 0.7}},
    {"glass_ball", {9, {-0.7, 0, 0.5}, {1.0, 1.0, 1.0, 1.0}, 0.5}},
    {"metal2_ball", {10, {-0.6, -0.3, 2}, {0.8, 0.6, 0.2, 1.0}, 0.2}}
  };

  renderer::Texture canvas_;
};
