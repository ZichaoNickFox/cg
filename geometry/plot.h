#pragma once

#include <algorithm>
#include <functional>
#include <matplot/matplot.h>
#include <vector>

#include "base/eigen_util.h"
#include "base/geometry.h"
#include "base/math.h"

namespace cg {

void Plot(int width, int height, std::vector<glm::vec4>* data, const glm::vec4& color, float scale,
          const std::vector<float>& xs, const std::function<float(float)>& transform_function) {
  float x_min = FLT_MAX;
  float x_max = FLT_MIN;
  std::vector<float> scaled_xs(xs.size());
  std::transform(xs.begin(), xs.end(), scaled_xs.begin(), [scale] (float x) { return x * scale; });
  for (float x : scaled_xs) {
    x_min = std::min(x_min, x);
    x_max = std::max(x_max, x);
  }
  std::vector<float> xs_plot = math::Linspace(x_min, x_max, 0.1);
  std::vector<float> ys_plot = math::Transform(xs_plot, transform_function);

  CGCHECK(data) << data;
  CGCHECK(xs_plot.size() == ys_plot.size()) << xs_plot.size() << " " << ys_plot.size();
  CGCHECK(width * height == data->size()) << width * height << " " << data->size();

  for (int i = 0, j = 1; j < xs_plot.size(); ++i, ++j) {
    // LineSegment l({xs_plot[i], ys_plot[i]}, {xs_plot[j], ys_plot[j]});
    // l.Bresenham(width, height, [data, width, color] (const glm::vec2& out) {
    //   int x = out.x;
    //   int y = out.y;
    //   int index = width * y + x;
    //   CGCHECK(index < data->size()) << index << " " << data->size();
    //   (*data)[index] = color;
    // });
  }
}

} // namespace cg