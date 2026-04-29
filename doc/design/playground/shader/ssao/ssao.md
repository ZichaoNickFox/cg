# playground/shader/ssao 设计

## 1. 组成
- `ssao_geometry.vs/fs.glsl`
- `ssao.cs.glsl`
- `ssao_ssao.cs.glsl`

## 2. 设计要点
- 目录同时保留较旧和当前命名的 SSAO compute shader，便于过渡与对照。
- 当前配置名 `ssao` 指向的是 `ssao_ssao.cs.glsl`，不是同名的 `ssao.cs.glsl`。
