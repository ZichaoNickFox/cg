# renderer/proto 设计

## 1. 组成
- `config.proto`
- `CMakeLists.txt`

## 2. 设计要点
- `CMakeLists.txt` 通过 `add_custom_command(...)` 调用 `protobuf::protoc` 生成代码。
- 生成代码被编入 `cg_proto`，供 `renderer`、`rhi` 与 `playground` 使用。
