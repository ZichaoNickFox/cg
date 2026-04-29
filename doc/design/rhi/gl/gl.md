# rhi/gl 设计

## 1. 组成
- `device.cc`
- `types.h`

## 2. 设计要点
- `OpenGLDevice`、`OpenGLBuffer`、`OpenGLVertexArray`、`OpenGLProgram` 都定义在 `device.cc` 中。
- `OpenGLDevice` 的公共 render state 提交面已经收口到 `ApplyRenderState()`；depth/cull/front-face 的逐项调用只保留成内部 helper。
- `OpenGLDevice` 也开始显式跟踪并兑现 `CaptureRenderState()` / `RestoreRenderState()`，让局部 render state 覆盖能通过 `ScopedRenderState` 自动回收。
- `OpenGLDevice` 的 viewport 也已经收口到 `RenderPassDesc` 与 `FramebufferState`；`glViewport` 只由 backend 内部 helper 驱动。
- `OpenGLDevice::ConfigureFramebuffer()` 负责把 `FramebufferDesc` 映射到 GL FBO attachment、read/draw buffer 和 completeness check。
- stencil attachment 现在使用独立的 stencil texture format 映射，而不是复用 depth texture 配置。
- `OpenGLDevice::ReadPixels()` 负责根据 `ReadPixelsDesc` 临时切换 read framebuffer/read buffer；对 color attachment 会显式切换 read buffer，对 depth/stencil attachment 则直接按绑定 framebuffer 读回，并以 tight pack 方式返回像素。
- `OpenGLDevice::ReadPixels()` 也会先校验 attachment 合法性：默认 framebuffer 只能读 attachment 0，非默认 framebuffer 则必须真实挂有对应 attachment，避免把调用方错误直接透传给 GL。
- `OpenGLDevice::BlitFramebuffer()` 负责根据 `BlitFramebufferDesc` 选择 source/destination attachment；当 color source 来自默认 framebuffer 时，会先切到 `read_buffer` 指定的 front/back；当 color destination 写回默认 framebuffer 时，也会切到 `draw_buffer` 指定的 front/back，再把 source/destination rect 原样映射到 `glBlitFramebuffer` 并在结束后恢复原 read/draw framebuffer；在切换 GL 状态前，它也会先校验 source/destination attachment 合法性。
- `OpenGLDevice::Clear()` 现在直接消费 `ClearDesc`，把 color/depth/stencil clear 值在一次调用内映射到 GL clear 状态，而不是要求上层先单独设置 clear color。
- `OpenGLDevice::BeginRenderPass()` 会先校验 `RenderPassDesc.viewport_size` 为正尺寸，并消费 `RenderPassDesc` 的 depth/stencil clear value，而不是只依赖 OpenGL 默认值。
- `OpenGLDevice::BeginRenderPass()` 在非默认 framebuffer 上执行 attachment-selective color clear 时，也会读取当前 draw framebuffer 绑定并校验目标 color attachment 是否真的存在，避免把非法 attachment index 直接透传给 GL。
- 这是当前功能最完整的后端，也是 Vulkan 路径的重要对照组。
