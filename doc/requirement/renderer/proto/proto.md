# renderer/proto 需求

## 1. 目标
本目录定义 renderer 资源配置的 protobuf 模型，并在构建时生成对应 C++ 代码。

## 2. 功能需求
- 应定义 `ShaderConfig`、`TextureConfig`、`ModelConfig`、`ConfigData`。
- 应支持 `protoc` 在构建时生成 `config.pb.cc/.h`。
- 不应在本目录中放置业务场景逻辑。
