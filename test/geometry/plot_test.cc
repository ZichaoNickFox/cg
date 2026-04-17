#include "gtest/gtest.h"

#include <vector>

#include "geometry/plot.h"
#include "imgui.h"
#include "implot.h"

namespace {

class ImGuiPlotContextGuard {
 public:
  ImGuiPlotContextGuard() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(1280.0f, 720.0f);
    io.DeltaTime = 1.0f / 60.0f;
    unsigned char* pixels = nullptr;
    int width = 0;
    int height = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
  }

  ~ImGuiPlotContextGuard() {
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
  }
};

TEST(geometry_plot_test, PlotBeginFalsePathLeavesImGuiStateBalanced) {
  ImGuiPlotContextGuard guard;

  ImGui::NewFrame();
  ImGui::SetNextWindowCollapsed(true, ImGuiCond_Always);
  EXPECT_FALSE(cg::PlotBegin("collapsed-plot"));
  ImGui::Render();

  ImGui::NewFrame();
  ASSERT_TRUE(cg::PlotBegin("visible-plot"));
  const std::vector<float> xs = {0.0f, 1.0f, 2.0f};
  const std::vector<float> ys = {0.0f, 1.0f, 4.0f};
  cg::PlotLine("samples", xs, ys, true);
  cg::PlotLine("transform", xs, [](float value) { return value * value; });
  cg::PlotEnd();
  ImGui::Render();
}

}  // namespace
