# test/test_support 需求

## 1. 目标
本目录提供单元测试复用的测试支撑对象。

## 2. 当前使用方
- `test/renderer/*`
- `test/rhi/rhi_logic_test.cc`

## 3. 功能需求
- 应提供 fake `rhi::Device`、fake renderer device 等测试替身。
- 测试支撑对象应能记录 buffer、program、render pass、render state 等关键调用信息。
- 应提供 RAII 风格的 `ScopedFakeDevice` / `ScopedFakeRendererDevice`，安全替换全局活动设备。
- 支撑代码不应反向依赖具体业务场景。
