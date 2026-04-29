# rhi/vulkan/shader 设计

## 1. 组成
- `texture_present.vert.glsl/.spv`
- `texture_present.frag.glsl/.spv`

## 2. 设计要点
- 目录内容服务 Vulkan presenter 的全屏纹理提交路径，而不是一般场景 shader。
- `presenter.cc` 只依赖 `.spv` 产物，因此源码与产物必须保持同名、同主题、可对应。
