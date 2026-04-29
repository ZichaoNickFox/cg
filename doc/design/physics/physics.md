# physics 设计

## 1. 角色
`physics/` 当前通过 `leap_frog.h` 暴露最小物理积分相关结构，并以 interface target `cg_physics` 形式被场景层引用。

## 2. 设计要点
- 目录当前实现非常薄，更多像“物理实验入口占位层”。
- 之所以保留单独目录，是为了让物理实验代码不与 renderer 或 geometry 工具混放。
- `playground/physics_scene` 通过该目录建立物理场景与基础数值模块的依赖关系。

## 3. 边界
- 当前不在此目录中实现完整物理世界、碰撞系统或求解器框架。
