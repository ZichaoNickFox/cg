#pragma once

#include <algorithm>
#include <functional>
#include "implot.h"
#include <vector>

#include "base/geometry.h"
#include "base/math.h"

namespace cg {

bool PlotBegin(const std::string& name, int width = 1000);
void PlotLine(const std::string& name, const std::vector<float>& xs, const std::vector<float>& ys,
              bool show_marker = false);
void PlotLine(const std::string& name, const std::vector<float>& xs,
              const std::function<float(float)>& transform_function, bool show_marker = false);
void PlotEnd();

} // namespace cg