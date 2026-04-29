# renderer/shader/ray_tracing 设计

## 1. 组成
- `ray_tracing.cs.glsl`
- `ray_tracing_canvas.fs.glsl`

## 2. 设计要点
- 目录按“光线追踪核心计算 + 结果展示”两段式组织。
- 当前正式注册场景 `RayTracingScene` 主要依赖 compute shader，canvas 片元 shader保留为展示辅助路径。
