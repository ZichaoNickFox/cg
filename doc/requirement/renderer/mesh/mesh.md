# renderer/mesh 需求

## 1. 目标
本目录负责基础网格构造、顶点布局约定和网格辅助工具。

## 2. 功能需求
- 应提供 `Mesh` 与 `MeshRepo` 支撑。
- 应支持位置、法线、纹理坐标、切线、副切线、颜色等常见顶点属性。
- 应提供 cube、plane、sphere、lines、empty mesh 等基础网格实现。
- 应能把 CPU 侧几何拆解为 `PrimitiveRepo` 所需的 primitive 数据。
