# rhi/vulkan/shader 需求

## 1. 目标
本目录保存 Vulkan presenter 使用的 shader 及其 SPIR-V 产物。

## 2. 当前使用方
- `rhi/vulkan/presenter.cc` 通过固定路径常量直接加载这里的 `.spv`
- `.glsl` 文件作为源码保留，用于重新编译或审阅

## 3. 功能需求
- 应同时保留源码 `.glsl` 与可直接加载的 `.spv`。
- 应覆盖 presenter 纹理展示所需的顶点与片元阶段。
- 若更新 `.glsl` 源码，应同步更新对应 `.spv` 产物。
