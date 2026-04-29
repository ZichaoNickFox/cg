# rhi/vulkan 需求

## 1. 目标
本目录实现 Vulkan 后端与 Vulkan presenter，服务 `vk` 和 `vk standalone` 运行路径。

## 2. 功能需求
- 在检测到 Vulkan SDK 时，应可被构建进工程。
- 应提供 Vulkan scene backend 和 Vulkan presenter。
- standalone 模式下应不依赖 offscreen OpenGL context。
- `standalone` 应作为 Vulkan scene renderer 的目标态，`compat` 作为迁移/回退态保留。
- standalone backend 应支持 `FramebufferDesc` 驱动的离屏 framebuffer 配置与完整性判定。
- Vulkan backend 应通过 `ApplyRenderState()` 消费公共 render state；depth/cull/front-face 的逐项 setter 不应继续作为上层常规接口暴露。
- Vulkan backend 也应支持 `CaptureRenderState()` / `RestoreRenderState()` 与 `ScopedRenderState`，用于收口局部 render state 覆盖后的恢复语义。
- Vulkan backend 也应通过 `RenderPassDesc` 或 `FramebufferState` 消费 viewport，而不是继续暴露单独的公有 viewport setter。
- standalone backend 应要求 `BeginRenderPass()` 的 `RenderPassDesc.viewport_size` 为正尺寸，而不是接受零或负的 viewport 宽高。
- standalone backend 应在 `BeginRenderPass()` 的非默认 framebuffer 路径上再次校验目标 framebuffer 已存在且 complete，而不是仅依赖调用方检查 `ConfigureFramebuffer()` 的返回值。
- standalone backend 应在 `BeginRenderPass()` 中校验 `RenderPassDesc.color_attachments` 的合法性：默认 framebuffer 只允许 attachment 0，非默认 framebuffer 只允许引用真实挂载的 color attachment。
- standalone backend 应在非默认 framebuffer render pass 中校验 `clear_depth` / `clear_stencil` 的目标附件存在性：请求 depth/stencil clear 时，depth/stencil attachment 0 必须真实挂载。
- standalone backend 应支持 `ClearDesc` 驱动的显式 clear 提交，在一次调用内消费 clear mask、color、depth clear value 与 stencil clear value，而不是依赖拆散的 clear 状态。
- standalone backend 应在 render pass 的非默认 framebuffer color clear 路径上校验目标 attachment 的存在性：默认 framebuffer 只允许 attachment 0，非默认 framebuffer 只允许引用真实挂载的 color attachment。
- standalone backend 应在 split read/draw framebuffer 绑定下让写侧语义跟随当前 draw framebuffer，包括 color attachment clear 与 framebuffer state capture，而不要求上层显式依赖单独的 `BindFramebuffer(...)` 公有接口。
- standalone backend 应在 framebuffer state capture/restore 中保留 split read/draw binding，而不是只记一个 framebuffer。
- standalone backend 应支持 `ReadPixelsDesc` 驱动的 default framebuffer color/depth/stencil 读回，以及离屏 color/depth/stencil attachment 读回。
- standalone backend 应在 `ReadPixelsDesc` 中校验目标 attachment 的存在性：默认 framebuffer 只允许 attachment 0，非默认 framebuffer 则只允许读取真实挂载的 color/depth/stencil attachment。
- 当 standalone backend materialize 默认 color framebuffer 时，应把 `ReadBuffer::kFront` 解释为上一份已完成的默认 framebuffer 结果，把 `ReadBuffer::kBack` 解释为当前默认 framebuffer 的活动写目标。
- 当 standalone backend 执行 color readback 且请求格式与底层 payload 不同，但仍属于常见 synthetic color 组合时，应优先保持请求侧的读回格式契约并完成必要的 payload 转换。
- standalone backend 应支持 `ProgramBindings.buffers` 在 `Program::ApplyBindings()` 调用内被兑现。
- standalone backend 也应接受通过 RHI convenience 入口组合提交的 `ProgramBindings + DrawDesc`、`ProgramBindings + ComputeDispatchDesc`，而不是要求这类“一次激活并提交”的语义只能停留在 renderer。
- standalone backend 应支持 `BlitFramebufferDesc` 驱动的 attachment-selective framebuffer blit，并尽可能保持默认 framebuffer 与离屏 depth/stencil 附件之间的复制语义。
- standalone backend 应在 `BlitFramebufferDesc` 中校验 source / destination attachment 的存在性：默认 framebuffer 只允许 attachment 0，非默认 framebuffer 则只允许引用真实挂载的 color/depth/stencil attachment。
- 当 standalone backend 以默认 framebuffer color 作为 blit source 时，应遵守 `BlitFramebufferDesc.read_buffer` 指定的 front/back 选择，而不是默认退化成单一 back buffer source。
- 当 standalone backend 以默认 framebuffer color 作为 blit destination 时，应遵守 `BlitFramebufferDesc.draw_buffer` 指定的 front/back 选择，并支持 default-to-default color blit。
- standalone backend 应支持 default-to-default depth/stencil blit，并保持这一默认路径不依赖 `read_buffer` / `draw_buffer`。
- standalone backend 应支持 `BlitFramebufferDesc.read_origin` / `draw_origin` 定义的带偏移 copy 语义，而不是只支持从 `(0, 0)` 到 `(0, 0)` 的 blit。
- 当 standalone backend 遇到负 `read_origin` 或负 `draw_origin` 时，应将其解释为部分越界 rect，并继续按同一 blit 映射裁剪有效区域，而不是直接拒绝该请求；这一语义也应覆盖 default framebuffer 尚未 materialize 时的惰性尺寸推导。
- standalone backend 应在 default framebuffer 与离屏 attachment 的双向 blit 中遵守 `BlitFramebufferDesc.size` / `draw_size` 定义的 source / destination rect 语义，并在 color path 上支持显式缩放 blit 以及 `FilterMode::kNearest` / `kLinear` 指定的采样语义。
- 当 standalone backend 的 source rect 或 destination rect 部分超出边界时，应按同一 blit 映射同步裁剪 source / destination 的有效区域，而不是把完整 source 重新缩放进被截断后的 destination。
- 当 standalone backend 执行 overlapping self color blit 时，应先保留 source 区域快照，避免原地 copy 污染后续 source 读取。
- 当 standalone backend 执行 overlapping self default depth/stencil blit 时，也应先保留 source 区域快照，避免原地 copy 污染后续 source 读取。
- 当 standalone backend 执行 color attachment blit 时，不应通过覆写 destination attachment 的缓存元信息来伪造结果；在常见 color 格式组合下，应优先保持 destination 格式契约并完成必要的 payload 转换。
- standalone backend 应支持 `RenderPassDesc` 指定的 depth clear value 落到 synthetic depth attachment。
- standalone backend 应支持 `RenderPassDesc` 指定的 stencil clear value 落到 synthetic stencil attachment，并在 stencil mask blit 时保持 attachment 复制语义。
- standalone backend 应在 `DrawDesc` 提交阶段校验 active render pass、active program、`DrawDesc.vertex_array`，以及 `DrawDesc.kind == kElements` 时的 `DrawDesc.index_buffer`；这些几何资源应直接由 `DrawDesc` 显式提供。
- standalone backend 也应允许上层先准备可复用的 `DrawDesc`，再在更高层与 program bindings 组合提交，而不是要求 draw 几何参数只能在最终 `Draw()` 调用点临时拼装。
- standalone backend 应在 `ComputeDispatchDesc` 提交阶段校验 active compute program、非 render pass 环境与非零 workgroup，并把 barrier 作为同一次 dispatch 描述的一部分消费。
- 目录中的 shader 资产应支撑 presenter 最终展示。
