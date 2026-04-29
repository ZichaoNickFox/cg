# renderer/shader/pbr 设计

## 1. 组成
- `pbr.vs/fs.glsl`
- `pbr_irradiance_*`
- `pbr_prefiltered_color_*`
- `pbr_BRDF_integration_map_generator.*`
- 多个 `pbr_*.glsl` 公共片段

## 2. 设计要点
- 目录同时承载运行时着色与离线/预处理 shader，是 PBR 相关逻辑的集中点。
- 顶层 shader 主要通过 `playground/shaders.cc` 中的 PBR 包装类被绑定到材质或工具场景。
