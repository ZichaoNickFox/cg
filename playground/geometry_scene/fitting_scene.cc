#include "playground/geometry_scene/fitting_scene.h"

#include <eigen/Dense>
#include <eigen/Sparse>

#include "base/color.h"
#include "base/debug.h"
#include "base/math.h"
#include "geometry/plot.h"
#include "renderer/shader.h"
#include "renderer/transform.h"

using namespace cg;

void FittingScene::OnEnter() {
  xs_ = std::vector<float>{1, 2, 3, 4, 5, 6};
  ys_ = std::vector<float>{5, 2, 4, 1, 3, 6};

  // PowerBasedFitting();
  // matplot::plot();
  using namespace matplot;
  std::vector<double> x = linspace(0, 2 * pi);
  std::vector<double> y = transform(x, [](auto x) { return sin(x); });

  plot(x, y, "-o");
  hold(on);
  plot(x, transform(y, [](auto y) { return -y; }), "--xr");
  plot(x, transform(x, [](auto x) { return x / pi - 1.; }), "-:gs");
  plot({1.0, 0.7, 0.4, 0.0, -0.4, -0.7, -1}, "k");

  // show();
}

void FittingScene::OnUpdate() {
}

void FittingScene::OnRender() {
  // FullscreenQuadShader fullscreen({texture_}, *this);
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

  int screen_width = io_->screen_size().x;
  int screen_height = io_->screen_size().y;
  data_.resize(screen_width * screen_height);
  Plot(screen_width, screen_height, &data_, kRed, 10, xs_, [&w, max_pow] (float x) {
    // float xn = 1;
    // float res = 0;
    // for (int i = 0; i <= max_pow; ++i) {
    //   res += xn * w[i];
    //   xn *= x;
    // }
    // return res;
    // return x * x;
    return x * x;
  });

  texture_ = CreateTexture2D(io_->screen_size().x, io_->screen_size().y, data_);
}