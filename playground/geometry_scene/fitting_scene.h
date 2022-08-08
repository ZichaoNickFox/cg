#pragma once

#include <vector>

#include <eigen/Core>
#include <glm/glm.hpp>

#include "renderer/camera.h"
#include "renderer/scene.h"

// https://github.com/Ubpa/GAMES102/tree/main/homeworks/hw1
class FittingScene : public cg::Scene {
 public:
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  void PowerBasedFitting();

  std::vector<float> xs_;
  std::vector<float> ys_;

  cg::Texture texture_;
  std::vector<glm::vec4> data_;
};