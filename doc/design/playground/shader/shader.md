# playground/shader 设计

## 1. 角色
本目录是 playground 场景专用 shader 目录，与 `renderer/shader/` 的通用 shader 库分工明确。

## 2. 分层
- 根目录：少量独立 shader，如 `sample_scene`、`geometry_scene`
- `rtrt/`：RTRT 混合路径相关 shader
- `shadow_scene/`：阴影场景 shader
- `ssao/`：SSAO 场景 shader

## 3. 设计要点
- 场景特化逻辑放这里，通用 include/库能力优先复用 `renderer/shader`。
- 目录里的文件多数会先在 `playground/config.pb.txt` 中注册为 shader 名，再由 `RenderShader`/`ComputeShader` 路径间接加载。
