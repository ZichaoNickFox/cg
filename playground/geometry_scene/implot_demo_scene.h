#pragma once

#include "renderer/scene.h"

// https://github.com/Ubpa/GAMES102/tree/main/homeworks/hw1
class ImPlotDemoScene : public cg::Scene {
 public:
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;
};