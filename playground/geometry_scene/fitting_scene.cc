#include "playground/geometry_scene/fitting_scene.h"

#include <eigen/Core>
#include <eigen/Dense>
#include <eigen/Sparse>
#include <numeric>

#include "base/color.h"
#include "base/debug.h"
#include "geometry/plot.h"
#include "renderer/shader.h"
#include "renderer/transform.h"

using namespace cg;

void FittingScene::OnEnter() {
  xs_ = std::vector<float>{1, 2, 3, 4, 5, 6};
  ys_ = std::vector<float>{5, 2, 4, 1, 3, 6};
}

void FittingScene::OnUpdate() {
}

void FittingScene::OnRender() {
  if (PlotBegin("fitting")) {
    PlotLine("source", xs_, ys_, true);
    PowerBaseInterpolation(xs_.size() - 1);
    GaussBaseInterpolation();
    LinearApproaching();
    QuadraticApproaching();
    PlotEnd();
  }
}

void FittingScene::OnExit() {
}

eigen::MatrixXf FittingScene::VendermondeMatrix(const std::vector<float>& xs, int max_power) {
  eigen::MatrixXf res(xs.size(), max_power + 1);
  for (int i = 0; i < xs_.size(); ++i) {
    float x = xs_[i];
    float v = 1;
    for (int j = 0; j <= max_power; ++j) {
      res(i, j) = v;
      v = v * x;
    }
  }
  return res;
}

void FittingScene::PowerBaseInterpolation(int max_power) {
  eigen::MatrixXf A = VendermondeMatrix(xs_, max_power);
  eigen::VectorXf b = math::Std2Eigen(ys_);
  eigen::VectorXf w = A.colPivHouseholderQr().solve(b);

  std::function f = [&w, max_power] (float x) {
    float xn = 1;
    float res = 0;
    for (int i = 0; i <= max_power; ++i) {
      res += xn * w[i];
      xn *= x;
    }
    return res;
  };

  std::vector<float> plot_xs = math::Linspace(math::Min(xs_), math::Max(xs_), 100);
  PlotLine("power", plot_xs, f);
}

void FittingScene::GaussBaseInterpolation(float sigma) {
  int N = xs_.size();
  eigen::MatrixXf A(xs_.size(), N + 1);
  std::function<float(float, float, float)> g_i = [] (float x, float x_i, float sigma) {
    return std::exp(-(pow((x - x_i), 2)) / (2 * sigma * sigma));
  };
  for (int i = 0; i < xs_.size(); ++i) {
    A(i, 0) = 1;
    for (int j = 1; j <= N; ++j) {
      A(i, j) = g_i(xs_[i], xs_[j - 1], 1);
    }
  }
  eigen::VectorXf b = math::Std2Eigen(ys_);
  eigen::VectorXf w = A.colPivHouseholderQr().solve(b);

  std::function<float(float)> f = [&w, &g_i, N, this] (float x) {
    eigen::VectorXf xs(N + 1);
    xs[0] = 1;
    for (int i = 1; i <= N; ++i) {
      xs[i] = g_i(x, xs_[i - 1], 1);
    }
    return xs.dot(w);
  };
  std::vector<float> plot_xs = math::Linspace(math::Min(xs_), math::Max(xs_), 100);
  PlotLine("gauss", plot_xs, f);
}

void FittingScene::LinearApproaching() {
  eigen::VectorXf xs = math::Std2Eigen(xs_);
  eigen::Matrix2f A;
  A(0, 0) = xs_.size();
  A(0, 1) = xs.sum();
  A(1, 0) = A(0, 1);
  A(1, 1) = xs.dot(xs);

  eigen::VectorXf ys = math::Std2Eigen(ys_);
  eigen::Vector2f b;
  b(0) = std::accumulate(ys_.begin(), ys_.end(), 0.0);
  b(1) = ys.dot(xs);

  eigen::Vector2f w = A.colPivHouseholderQr().solve(b);

  std::function<float(float)> f = [w] (float x) {
    return w[0] + w[1] * x;
  };
  std::vector<float> plot_xs = math::Linspace(math::Min(xs_), math::Max(xs_), 100);
  PlotLine("linear", plot_xs, f);
}

void FittingScene::QuadraticApproaching() {
  eigen::VectorXf xs = math::Std2Eigen(xs_);
  eigen::VectorXf x2s = math::Std2Eigen(xs_).unaryExpr([] (float x) { return x * x; });

  eigen::Matrix3f A;
  A(0, 0) = xs.size();
  A(0, 1) = xs.sum();
  A(0, 2) = x2s.sum();
  A(1, 0) = A(0, 1);
  A(1, 1) = xs.dot(xs);
  A(1, 2) = xs.dot(x2s);
  A(2, 0) = A(0, 2);
  A(2, 1) = A(1, 2);
  A(2, 2) = x2s.dot(x2s);

  eigen::VectorXf ys = math::Std2Eigen(ys_);
  eigen::Vector3f b;
  b(0) = std::accumulate(ys_.begin(), ys_.end(), 0.0);
  b(1) = ys.dot(xs);
  b(2) = ys.dot(x2s);

  eigen::Vector3f w = A.colPivHouseholderQr().solve(b);

  std::function<float(float)> f = [w] (float x) {
    return w[0] + w[1] * x + w[2] * x * x;
  };
  std::vector<float> plot_xs = math::Linspace(math::Min(xs_), math::Max(xs_), 100);
  PlotLine("quadratic", plot_xs, f); 
}