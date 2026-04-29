# playground/physics_scene 需求

## 1. 目标
本目录承载物理实验场景，当前主要场景为 `AngryBunnyScene`。

## 2. 功能需求
- 应提供继承 `cg::Scene` 的物理实验场景实现。
- 场景应复用 `physics/` 提供的基础能力，并可结合 renderer 做可视化。
- 场景应可被 `Playground` 注册并通过 UI 进入。

## 3. 验收要点
- `AngryBunnyScene` 可在支持其能力要求的设备上进入并运行。
