#include "playground/geometry_scene/implot_demo_scene.h"

#include <implot.h>

using namespace cg;

void ImPlotDemoScene::OnEnter() {
}

void ImPlotDemoScene::OnUpdate() {
}

void ImPlotDemoScene::OnRender() {
  bool open = true;
  ImPlot::ShowDemoWindow(&open);
}

void ImPlotDemoScene::OnExit() {
}