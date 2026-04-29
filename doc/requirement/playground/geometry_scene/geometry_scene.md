# playground/geometry_scene 需求

## 1. 目标
本目录承载几何与数值可视化场景，当前为 `FittingScene` 和 `ImPlotDemoScene`。

## 2. 功能需求
- 应提供可被 `Playground` 注册和实例化的几何类场景。
- 场景应复用 `geometry/plot`、`renderer/scene` 与 ImGui/ImPlot 能力。
- `FittingScene` 应支撑曲线拟合、插值或数值展示类实验。
- `ImPlotDemoScene` 应作为 ImPlot 功能参考与回归入口。

## 3. 验收要点
- 场景可被 `Playground` 场景目录识别并进入。
- 场景在无 compute/storage buffer 前提下仍应可运行。
