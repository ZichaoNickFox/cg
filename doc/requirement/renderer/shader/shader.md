# renderer/shader 需求

## 1. 目标
本目录保存跨场景复用的通用 GLSL 代码和 shader 库。

## 2. 当前使用方式
- 大量条目通过 `playground/config.pb.txt` 注册为 shader 名
- `renderer/shader.cc` 中的 `RenderShader` / `ComputeShader` 包装类会间接消费这些配置名
- 一部分 shader 服务正式注册场景，一部分服务未注册但仍保留的实验或生成工具路径

## 3. 功能需求
- 应覆盖基础渲染、法线、纹理、阴影、PBR、路径追踪、随机采样等主题。
- 应允许通过 include 风格的公共 `.glsl` 文件复用共享结构和函数。
- 主题性更强的 shader 应进入对应子目录。
- 更偏场景特化的 shader 不应无限堆进本目录，而应优先放在 `playground/shader/`。
- compute shader 的上层触发应优先通过 renderer 的统一包装提交，而不是散落直接后端 dispatch。
- render shader 的高层资源提交也应优先通过 batched bindings 与统一包装入口完成，而不是继续扩散成碎片化的 `Use + named uniform` 手工调用。
- `renderer/shader.*` 中共享的 camera/model/frame-state 绑定逻辑应尽量沉淀为可复用 helper，供 renderer shader 与 playground scene 共同拼装单次 bindings 提交。
- 对 render shader 中最常见的 object draw 初始化，`renderer/shader.*` 还应提供可复用的 render-object helper，把 `model + camera` 组合提交收口成统一模板。
- 对仍使用 legacy `model/view/project` 与 `view_pos_ws` uniform 约定的 shader，`renderer/shader.*` 也应提供对应 helper，并要求 wrapper 继续通过 batched bindings 一次性兑现这组旧约定。

## 4. 子目录职责
- `bxdf/`：BRDF/BXDF 公式
- `lut/`：查找表与其生成脚本
- `pbr/`：PBR 着色与预计算 shader
- `phong/`：Phong 着色 shader
- `ray_tracing/`：光线追踪 compute/canvas shader
