# rhi/gl 需求

## 1. 目标
本目录实现 OpenGL 后端，是当前最稳定的场景渲染基线。

## 2. 功能需求
- 应完整实现 `rhi::Device` 所要求的主要渲染、资源、readback 和状态接口。
- 应支撑绝大多数正式场景的运行。
- 应与 renderer 层的 program bindings、framebuffer、texture 上传等逻辑兼容。
- 应通过 `ApplyRenderState()` 消费公共 render state，而不是要求上层继续逐项驱动 depth/cull/front-face setter。
- 也应支持 `CaptureRenderState()` / `RestoreRenderState()` 与 `ScopedRenderState`，用于收口局部 render state 覆盖后的恢复。
- 应通过 `RenderPassDesc` 或 `FramebufferState` 消费 viewport，而不是要求上层继续单独调用 viewport setter。
- 应负责将 `FramebufferDesc` 转换为 OpenGL framebuffer attachment、draw buffer 和完整性检查流程。
- depth attachment 与 stencil attachment 应映射到各自匹配的 OpenGL texture/internal format，而不是混用同一占位格式。
- 应负责将 `ReadPixelsDesc` 转换为 OpenGL read framebuffer / read buffer 选择，并支持从用户 framebuffer 的 color/depth/stencil attachment 输出紧凑排列的像素数据。
- 应负责将 `BlitFramebufferDesc` 转换为 OpenGL read/draw framebuffer 与 attachment-selective blit 流程；当 source 或 destination 为默认 framebuffer color 时，也应分别遵守 `read_buffer` / `draw_buffer` 的 front/back 选择，并将 source/destination rect 显式传递给 `glBlitFramebuffer`。
- 应通过 `ClearDesc` 在一次调用内消费 color/depth/stencil clear 信息，而不是继续依赖 `SetClearColor()` 一类拆散的 OpenGL 风格设备状态。
- 应根据 `RenderPassDesc` 中提供的 depth/stencil clear value 执行清理，而不是将这两个值固定写死。
- 应在 `BeginRenderPass()` 中拒绝零或负的 `RenderPassDesc.viewport_size`，避免将无效 viewport 直接透传给 OpenGL。
- 应在 render pass 的非默认 framebuffer color clear 路径上校验当前 draw framebuffer 的 color attachment 合法性：默认 framebuffer 只允许 attachment 0，非默认 framebuffer 则只允许真实挂载的 color attachment。
- 应在 `ReadPixels()` 中校验目标 attachment 合法性：默认 framebuffer 只允许 attachment 0，非默认 framebuffer 则只允许真实挂载的 color/depth/stencil attachment。
- 应在 `BlitFramebuffer()` 中校验 source / destination attachment 合法性：默认 framebuffer 只允许 attachment 0，非默认 framebuffer 则只允许真实挂载的 color/depth/stencil attachment。
