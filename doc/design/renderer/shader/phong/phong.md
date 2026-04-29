# renderer/shader/phong 设计

## 1. 组成
- `phong.vs.glsl`
- `phong.fs.glsl`
- `phong_model.glsl`

## 2. 设计要点
- 保持结构简单，作为非 PBR 基线路径的共享 shader。
- 这是当前正式注册场景目录中最直接在用的传统光照 shader 目录之一。
