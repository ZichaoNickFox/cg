#pragma once

#include "renderer/camera.h"
#include "renderer/scene.h"

class FittingScene : public renderer::Scene {
 public:
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  void power_base_fitting();
};