# playground/shader/shadow_scene 设计

## 1. 组成
- 主渲染：`shadow_scene.vs/fs.glsl`
- 深度 pass：`shadow_scene_depth.vs/fs.glsl`
- 公共逻辑：`shadow_scene_common.glsl`、`shadow_scene_pcf.glsl`、`shadow_scene_pcss.glsl`

## 2. 设计要点
- 把采样策略与主 shader 拆分，便于比较不同阴影过滤方案。
- `shadow_scene.fs.glsl` 通过 include 形式显式拼装公共逻辑和 PCF/PCSS 实现。
