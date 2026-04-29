# rhi/vulkan 设计

## 1. 组成
- `device.cc`：Vulkan `Device` 实现
- `presenter.cc/.h`：Vulkan 屏幕展示层
- `shader/`：presenter 使用的 GLSL/SPIR-V

## 2. 设计要点
- `VulkanDevice` 负责 scene backend 语义。
- standalone `VulkanDevice` 目前用 synthetic attachment record 实现 `FramebufferDesc`、blit 和 readback，而不是完整 Vulkan raster pipeline。
- Vulkan backend 的公共 render state 提交面也已经收口到 `ApplyRenderState()`；逐项 depth/cull/front-face 调整退回为 backend 内部 helper，compat 转发也通过局部 `RenderStateDesc` 进行。
- 在此之上，standalone / compat 两条路径也都开始兑现 `CaptureRenderState()` / `RestoreRenderState()`；对上层来说，临时 render state 覆盖现在可以直接通过 `ScopedRenderState` 回收，而不必再手工恢复。
- Vulkan backend 的 viewport 也已经收口到 `RenderPassDesc` 与 `FramebufferState`；standalone/default buffer sizing 仍会在 backend 内部 helper 中消化 viewport 变化。
- standalone `VulkanDevice` 里的 draw/read framebuffer 绑定已经开始分离；写侧语义和 framebuffer state capture 以当前 draw framebuffer 为准，避免 `kDraw` 绑定后仍误写旧目标。
- `FramebufferState` 在 standalone 模式下会显式保留 read/draw framebuffer；旧的单一 framebuffer 字段保留为 draw alias，用于兼容现有上层调用点。
- standalone `VulkanDevice::Clear()` 现在直接消费 `ClearDesc`，在一次调用内把 clear mask、clear color、depth clear value 与 stencil clear value materialize 到 synthetic 默认缓冲或当前 draw framebuffer 附件，而不是再依赖分裂的 clear 状态。
- `ReadPixelsDesc` 在 standalone 模式下直接映射到 synthetic default color/depth/stencil buffer 或指定 color/depth/stencil attachment 的 CPU 读回；对于常见 synthetic color 组合，color readback 会按请求格式执行 CPU 侧 payload 转换。
- `ProgramBindings` 中的 `buffers` 在 standalone 模式下也会由 `Program::ApplyBindings()` 统一兑现，不再依赖上层补发独立 buffer-binding 调用。
- 在这条基础上，standalone/Vulkan 路径也自然接受由 RHI convenience 入口组合提交的 `ProgramBindings + DrawDesc`、`ProgramBindings + ComputeDispatchDesc`，因为最终仍会落到同一套 `Program::ApplyBindings()` + draw/dispatch 状态机。
- standalone 的默认 framebuffer 现在会 materialize color/depth/stencil 三类 synthetic buffer；其中 color 又拆成 front/back 两份：back 作为当前默认 framebuffer 的活动写目标，front 作为上一份已完成结果，并在默认 render pass 结束时跟上 back。
- standalone `BeginRenderPass()` 的第一层守门现在是 viewport：只有正尺寸 `RenderPassDesc.viewport_size` 才允许进入 pass，避免把零/负 viewport 继续传给 synthetic default buffer sizing 和后续 draw 生命周期。
- standalone `BeginRenderPass()` 在打开活动 pass 前，会先确认非默认 framebuffer 已存在且 complete；这样即使上层忽略了 `ConfigureFramebuffer(false)`，draw 生命周期也不会建立在不完整 attachment 集合上。
- standalone `BeginRenderPass()` 也会验证 `RenderPassDesc.color_attachments` 是否与目标 framebuffer 匹配：默认 framebuffer 只能声明 attachment 0，非默认 framebuffer 则必须真的挂有对应的 color attachment。
- 同一套守门也扩展到了 depth/stencil clear：当 non-default pass 请求 `clear_depth` 或 `clear_stencil` 时，standalone backend 会先确认 depth/stencil attachment 0 已存在，再继续进入 pass。
- non-default render pass 上的 attachment-selective color clear 也跟随 draw framebuffer 语义收紧：默认 framebuffer 只能清 attachment 0，非默认 framebuffer 则必须真的挂有对应的 color attachment，不能再静默忽略缺失索引。
- `ReadPixelsDesc` 在 standalone 模式下除了驱动 synthetic default/attachment readback，也会先验证 attachment 合法性：默认 framebuffer 只能读 attachment 0，非默认 framebuffer 则必须真实挂有对应 attachment，避免 silent zero-fill 掩盖调用方错误。
- `BlitFramebufferDesc` 在 standalone 模式下直接映射到 source / destination attachment 的 CPU 级复制，并支持默认 framebuffer 与离屏 depth/stencil 附件之间的同步；当 color source 来自默认 framebuffer 时，`read_buffer` 会决定使用 front 还是 back synthetic color buffer；当 color destination 写回默认 framebuffer 时，`draw_buffer` 会决定写 front 还是 back，并支持 default-to-default color copy；depth/stencil 也支持 default-to-default copy，但不依赖 front/back 选择；在真正执行 copy 前，source / destination attachment 也会先做同一套合法性校验，避免非法 attachment index 再静默退化成 no-op；`read_origin` / `draw_origin` 与 `size/draw_size` 会共同参与 synthetic copy 的偏移与缩放计算。
- `BlitFramebufferDesc.size` / `draw_size` 会约束 synthetic blit 的 source / destination rect；当 color path 的 source / destination size 不同时，standalone backend 会按 `filter` 执行 nearest/linear 风格的 CPU 侧采样，而 exact-format 的非 color payload 继续走 nearest-style payload 复制。负 `read_origin` / `draw_origin` 会先被解释为“requested rect 与 framebuffer 发生部分越界”；随后如果 source rect 或 destination rect 仍有可见区域，standalone backend 会按同一映射同时缩减 source / destination 的有效区域，而不是只截断 destination 再重采样完整 source。default framebuffer 若尚未 materialize，则会先按这套可见区域结果推导最小 synthetic buffer 尺寸，再执行真正的 copy。
- 当 color source / destination 落在同一份 synthetic 存储且复制区域重叠时，standalone backend 会先快照 source 区域，再执行 destination 写回，避免 `memcpy` 式原地覆写导致的 source 污染；default depth/stencil 的 overlapping self-blit 也遵循同样的快照策略。
- 当 source / destination color attachment 格式不同但属于常见 synthetic color 组合时，standalone blit 会按目标格式转换 payload，而不是覆写目标 attachment 的缓存元信息。
- `RenderPassDesc.depth_clear_value` 和 `stencil_clear_value` 会分别驱动 synthetic depth/stencil attachment 的 clear 结果。
- 当 `BlitFramebufferDesc.mask` 包含 stencil 时，standalone backend 也会复制 stencil attachment 的 CPU 存储。
- standalone draw 目前仍不是“真实 raster 执行”，但已经收紧为显式 graphics submission 状态机：`BeginRenderPass()` 打开活动 pass，`Program::ApplyBindings()` 负责记录活动 program，而 `DrawDesc + Draw()` 则直接消费描述对象里显式携带的 `vertex_array/index_buffer`。
- standalone compute 目前仍不是“真实 Vulkan compute pipeline”，但已经开始区分 render / compute program，并要求 `ComputeDispatchDesc` 只能在活动 compute program、非 render pass 环境和非零 workgroup 下执行；barrier 也跟随同一次 dispatch 描述被记录。
- `Presenter` 负责 swapchain 和纹理展示，不直接承担场景业务逻辑。
- 该目录只在 `find_package(Vulkan)` 成功时参与构建。
