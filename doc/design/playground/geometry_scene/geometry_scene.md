# playground/geometry_scene 设计

## 1. 组成
- `fitting_scene.*`
- `implot_demo_scene.*`

## 2. 设计要点
- 本目录通过 `cg_geometry_scene` 静态库对外暴露。
- 两个场景都继承自 `cg::Scene`，复用统一生命周期与 inspector。
- 该目录更偏“几何/数值实验”，比 `renderer_scene` 更少依赖复杂 GPU 特性。

## 3. 边界
- 不在本目录中实现运行时窗口或场景目录管理。
