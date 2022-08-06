#pragma once

#include <vector>

#include <eigen/Core>
#include "renderer/camera.h"
#include "renderer/scene.h"

class FittingScene : public renderer::Scene {
 public:
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  void PowerBasedFitting();

  std::vector<float> xs_;
  std::vector<float> ys_;
};