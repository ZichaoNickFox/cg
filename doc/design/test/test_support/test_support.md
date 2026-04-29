# test/test_support 设计

## 1. 组成
- `fake_device.h`
- `fake_renderer_device.h`

## 2. 设计要点
- `FakeDevice` 提供可观测的 `Buffer`、`VertexArray`、`Program`、`Device` 替身和状态记录结构。
- 其中 `FakeVertexArrayState` 需要记录 `VertexArrayBindingDesc` 这类描述式提交，避免测试只能观察旧的逐项 attribute setter。
- `FakeRendererDevice` 在 `FakeDevice` 基础上补齐 texture 上传/读回这类 renderer 更关心的行为。
- `ScopedFakeDevice` / `ScopedFakeRendererDevice` 通过 RAII 管理全局活动设备的注入与清理。
- 该目录的核心价值是把“断言渲染语义”从“依赖真实图形后端”中解耦出来。
