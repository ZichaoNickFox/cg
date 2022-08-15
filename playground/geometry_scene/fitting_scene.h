#pragma once

#include <vector>

#include <eigen/Core>
#include <glm/glm.hpp>

#include "base/math.h"
#include "renderer/camera.h"
#include "renderer/scene.h"

// https://github.com/Ubpa/GAMES102/tree/main/homeworks/hw1
// http://staff.ustc.edu.cn/~lgliu/Courses/GAMES102_2020/default.html
class FittingScene : public cg::Scene {
 public:
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  void PowerBaseInterpolation(int max_power);
  void GaussBaseInterpolation(float sigma = 1);
  void LinearApproaching();
  void QuadraticApproaching();

  eigen::MatrixXf VendermondeMatrix(const std::vector<float>& xs, int max_power);

  std::vector<float> xs_;
  std::vector<float> ys_;
};