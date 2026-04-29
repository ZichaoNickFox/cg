# cmake 需求

## 1. 目标
`cmake/` 目录负责本工程的构建辅助逻辑，确保顶层 `CMakeLists.txt` 能稳定发现依赖、配置第三方库并识别可选运行时能力。

## 2. 范围
- 第三方依赖接入与一致化命名
- `MesaEGL` / `OSMesa` 的查找逻辑
- 与 `CG_ENABLE_VULKAN`、`CG_ENABLE_OSMESA` 等开关配套的配置支持

## 3. 功能需求
- 应在配置阶段校验 `thirdparty/` 中的关键依赖是否存在，并在缺失时给出明确错误。
- 应把 `glog`、`gtest`、`protobuf` 等目标统一成工程内部使用的别名。
- 应支持检测 `OSMesa` 与 `MesaEGL`，并把检测结果回传给主工程用于启用或关闭 offscreen runtime。
- 不应在该目录中引入场景逻辑、renderer 逻辑或运行时业务代码。

## 4. 验收要点
- `cmake --preset dev` 在依赖完整时可完成配置。
- 缺失关键第三方依赖时，配置阶段直接失败且错误信息可读。
- 有无 Mesa 的环境下，配置结果能正确反映到 `CG_HAS_*` 相关变量。
