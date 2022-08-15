#include "geometry/plot.h"

#include <implot.h>
#include <implot_internal.h>

#include "base/debug.h"

namespace cg {

bool PlotBegin(const std::string& name, int width) {
  ImGui::SetNextWindowSize(ImVec2(width, 0), ImGuiCond_FirstUseEver);
  ImGui::Begin(name.c_str());
  return ImPlot::BeginPlot(name.c_str());
}

void PlotLine(const std::string& name, const std::vector<float>& xs, const std::vector<float>& ys,
              bool show_marker) {
  if (show_marker) {
    ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
  }
  ImPlot::PlotLine(name.c_str(), xs.data(), ys.data(), xs.size());
}

void PlotLine(const std::string& name, const std::vector<float>& xs,
              const std::function<float(float)>& transform_function, bool show_marker) {
  std::vector<float> ys(xs.size());
  std::transform(xs.begin(), xs.end(), ys.begin(), transform_function);
  PlotLine(name.c_str(), xs, ys, show_marker);
}

void PlotEnd() {
  ImPlot::EndPlot();
  ImGui::End();
}

} // namespace cg