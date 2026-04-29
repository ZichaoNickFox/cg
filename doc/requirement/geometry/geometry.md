# geometry 需求

## 1. 目标
`geometry/` 目录承载与几何绘图/可视化相关的轻量能力，当前重点是 ImPlot 画图辅助。

## 2. 范围
- 统一的 ImPlot 启停与折线绘制辅助

## 3. 功能需求
- 应提供统一的 `PlotBegin` / `PlotLine` / `PlotEnd` 接口，减少场景直接操作 ImPlot 的样板代码。
- 应同时支持直接传入 `x/y` 序列，以及传入 `x` 序列加变换函数的绘图方式。
- 该目录能力应主要服务几何与数值实验场景。

## 4. 依赖与约束
- 允许依赖 `ImPlot`、`base` 的数学和几何工具。
- 不应承担 renderer、RHI 或场景目录管理职责。

## 5. 验收要点
- `test/geometry/plot_test.cc` 应能验证基本绘图辅助行为。
