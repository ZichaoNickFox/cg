#include "playground/geometry_scene/fitting_scene.h"

#include <eigen/Dense>
#include <eigen/Sparse>

#include "base/math.h"
#include "geometry/plot.h"
#include "renderer/shader.h"
#include "renderer/transform.h"

void FittingScene::OnEnter() {
  xs_ = std::vector<float>{1, 2, 3, 4, 5, 6};
  ys_ = std::vector<float>{5, 2, 4, 1, 3, 6};

  PowerBasedFitting();
}

void FittingScene::OnUpdate() {
}

void FittingScene::OnRender() {
}

void FittingScene::OnExit() {
}

void FittingScene::PowerBasedFitting() {
  int max_pow = 3;
  eigen::MatrixXf A(xs_.size(), max_pow + 1);
  for (int i = 0; i < xs_.size(); ++i) {
    float x = xs_[i];
    float v = 1;
    for (int j = 0; j <= max_pow; ++j) {
      A(i, j) = v;
      v = v * x;
    }
  }
  eigen::VectorXf b(ys_.size());
  for (int i = 0; i < ys_.size(); ++i) {
    float y = ys_[i];
    b(i) = y;
  }
  eigen::VectorXf w = A.colPivHouseholderQr().solve(b);

  geometry::Plot(xs_, [&w, max_pow] (float x) {
    float xn = 1;
    float res = 0;
    for (int i = 0; i <= max_pow; ++i) {
      res += xn * w[i];
      xn *= x;
    }
    return res;
  });
}