# playground/physics_scene 设计

## 1. 组成
- `angry_bunny_scene.*`

## 2. 设计要点
- 目录通过 `cg_physics_scene` 静态库暴露。
- 当前只有一个正式场景，因此目录形态非常扁平。
- 该场景通过 `cg::Scene` 接入统一相机、输入、资源 repo 和 inspector。

## 3. 边界
- 物理基础算法依旧放在 `physics/`，本目录更偏场景装配与可视化。
