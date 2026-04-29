# renderer/shader 设计

## 1. 角色
本目录是 renderer 层的通用 shader 库，与 `playground/shader/` 的场景私有 shader 相区分。

## 2. 设计要点
- 根目录保存跨主题复用的公共定义、基础 shader 和 include 文件。
- 主题性较强的 shader 被拆到 `bxdf`、`pbr`、`phong`、`ray_tracing` 等子目录。
- `lut/` 同时保存生成用脚本和产出的 `.glsl` 数据文件。
- `playground/config.pb.txt` 将这里的文件路径映射成上层可引用的 shader 名，例如 `phong`、`pbr`、`ray_tracing`。
- renderer 高层的 compute 路径开始通过基于 `ComputeDispatchDesc` 的 `ShaderProgram::DispatchCompute()` 与 `ShaderProgram::DispatchComputeBindings()` 收口 program 激活、bindings 提交和 dispatch 下发；更上一层的 `ComputeShader` 包装也开始提供一次性 dispatch helper，把 scene wrapper 从 `SetWorkGroupNum() + Run()` 这类状态机式调用迁走。
- renderer 高层的 render 路径也开始通过 batched `ShaderProgramBindings`、`ShaderProgram::ApplyBindings()` 和 `ShaderProgram::DrawBindings()` 收口高频 program 提交，减少 camera/material/common param 这类高频 named-uniform 提交的碎片化调用，并把 bindings + draw 两步常见组合继续收回到单次入口。
- 与此配套，`renderer/shader.cc` 也把 camera/model/material index/resolution/frame num/dirty 这些高频提交项整理成 append helpers，让 renderer 自带 shader 包装和 playground scene shader 都能用同一套 bindings 构建方式；其中 `model + camera` 还继续被组合成 render-object helper，legacy `model/view/project/view_pos_ws` 约定则由另一组 helper 承接，专门服务尚未迁完的矩阵式 shader。
- 当前 `renderer/shader.*` 自带的 render/compute wrapper 也已经对齐到这套提交模型：不再继续暴露 stateful setter，而是直接在构造阶段缓存 scene-common bindings，并在真正 draw/dispatch 时与局部 bindings 合并后一次性下发；对明确会立刻 draw 的 render wrapper，则优先通过 `DrawBindings()` 把 bindings 与 draw 提交一并下发。

## 3. 边界
- 场景专项 shader 优先放到 `playground/shader/`，而不是无限膨胀本目录。
