# playground/resource/model/cornell_box 设计

## 1. 组成
目录按部件拆分为 `floor`、`left`、`right`、`light`、`short_box`、`tall_box` 等 OBJ/MTL 对。

## 2. 设计要点
- 使用分件而不是单一总模型，便于场景实验单独赋材质、发光或几何属性。
- `PathTracingScene` 和 `RTRTScene` 在头文件中显式按部件拼装该场景。
- 目录组织因此围绕“可独立引用的部件”而不是“一个总模型文件”展开。
