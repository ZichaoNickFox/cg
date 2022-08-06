#pragma once

#include <vector>
#include <functional>
// #include <matplot/matplot.h>

#include "base/math.h"
#include "base/eigen_util.h"

namespace geometry {

void Plot(const std::vector<float>& xs, const std::function<float(float)>& transform_function) {
  float x_min = FLT_MAX;
  float x_max = FLT_MIN;
  for (float x : xs) {
    x_min = std::min(x_min, x);
    x_max = std::max(x_max, x);
  }
  std::vector<float> xs_plot = math::Linspace(x_min, x_max, 0.1);
  std::vector<float> ys_plot = math::Transform(xs_plot, transform_function);

  // matplot::plot(xs_plot, ys_plot, "-o");
  // matplot::show();
}

} // namespace geometry
