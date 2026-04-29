# test/rhi 需求

## 1. 目标
本目录验证 RHI 抽象及其后端实现的关键行为。

## 2. 当前测试目标
- `rhi_logic_test`
- `rhi_vulkan_test`（仅当 Vulkan target 存在时）

## 3. 功能需求
- 应覆盖 RHI 公共逻辑。
- `rhi_logic_test` 应覆盖 `ClearDesc` 这类抽象层显式提交契约，确保 clear 参数不会重新退回到隐式设备状态。
- `rhi_logic_test` 还应覆盖 `VertexArrayBindingDesc` 这类配置型描述对象的 forwarding/capture 语义，确保 vertex attribute 上传不会重新退回业务层手工 attribute setter。
- `rhi_logic_test` 还应覆盖 `Device::DrawBindings()` / `Device::DispatchComputeBindings()` 这类 convenience 提交，确保 bindings 与 draw/dispatch 的组合语义不会只散落在更高层包装里。
- `rhi_logic_test` 还应覆盖 `Device::ApplyBufferBindings()` 的 forwarding/capture 语义，确保独立 buffer base 绑定不会重新退回业务层散落 `BindBase()`。
- `rhi_logic_test` 也应覆盖 direct `BindBase()` 不会被误记成 descriptor 级 `ApplyBufferBindings()` 提交，避免测试语义再次混淆。
- `rhi_logic_test` 还应覆盖 `BufferReadDesc` 这类 host readback 描述对象的 forwarding/capture 语义，确保 buffer 读回不会重新退回裸 `Map()/Unmap()`。
- 当 Vulkan 后端被构建时，应补充 Vulkan 设备相关测试。
- 当 Vulkan standalone 被构建时，应覆盖 draw 前置状态边界，包括 render pass、program、`DrawDesc.vertex_array` 与 `DrawDesc.index_buffer` 约束，并至少包含一条 `DrawDesc` 直接携带 geometry binding 的正向用例。
- `rhi_logic_test` 还应覆盖 scoped render pass helper 是否会在生命周期内自动完成 begin/end 配对。
- `rhi_logic_test` 还应覆盖 scoped framebuffer state helper 是否会在生命周期结束时恢复先前的 framebuffer state。
- `rhi_logic_test` 还应覆盖 scoped render state helper 是否会在生命周期结束时恢复先前的 depth/cull/front-face 状态。
- 当 Vulkan standalone 被构建时，也应覆盖 `CaptureRenderState()` / `RestoreRenderState()` 的正向 round-trip，用于守住 standalone scene backend 的 render state 跟踪实现。
- 当 Vulkan standalone 被构建时，应覆盖 `BeginRenderPass()` 对零或负 viewport size 的拒绝路径。
- 当 Vulkan standalone 被构建时，应覆盖 `BeginRenderPass()` 对不完整非默认 framebuffer 的拒绝路径。
- 当 Vulkan standalone 被构建时，应覆盖 `BeginRenderPass()` 对非法 `RenderPassDesc.color_attachments` 的拒绝路径，包括 default framebuffer 的非法 attachment index，以及 non-default framebuffer 上未挂载的 color attachment。
- 当 Vulkan standalone 被构建时，应覆盖 non-default framebuffer render pass 在缺失 depth/stencil attachment 时对 `clear_depth` / `clear_stencil` 的拒绝路径。
- 当 Vulkan standalone 被构建时，应覆盖 `ReadPixels()` 对非法 attachment index 的拒绝路径，包括 default framebuffer 的非法 attachment index，以及 non-default framebuffer 上未挂载的 attachment。
- 当 Vulkan standalone 被构建时，应覆盖 `BlitFramebuffer()` 对非法 source / destination attachment 的拒绝路径，包括 default framebuffer 的非法 attachment index，以及 non-default framebuffer 上未挂载的 attachment。
- 当 Vulkan standalone 被构建时，应覆盖 `ComputeDispatchDesc` 的前置状态边界，包括 compute program、render pass、workgroup 与 barrier 描述收口约束。
- 当 Vulkan standalone 被构建时，应覆盖 `ProgramBindings.buffers` 在 `Program::ApplyBindings()` 内的兑现语义。
- 当 Vulkan standalone 被构建时，应覆盖 split read/draw framebuffer 绑定对写侧语义的影响，包括 framebuffer state capture 与 color attachment clear 目标。
- 当 Vulkan standalone 被构建时，应覆盖 framebuffer state capture/restore 在 split read/draw binding 下的保持语义。
- 当 Vulkan standalone 被构建时，应覆盖 `BlitFramebufferDesc.read_origin` / `draw_origin` 的 offset blit 语义，包括离屏 attachment 与 default framebuffer 路径。
- 当 Vulkan standalone 被构建时，应覆盖 stencil attachment 的 clear / blit 基本语义。
- 当 Vulkan standalone 被构建时，应覆盖 `ReadPixelsDesc` 对 color/depth/stencil attachment 读回语义的基本验证。
- 当 Vulkan standalone 被构建时，应覆盖至少一种 color readback 格式转换，用于验证 `ReadPixelsDesc` 的请求格式契约。
- 当 Vulkan standalone 被构建时，应验证默认 framebuffer 的 depth/stencil clear、readback 与 blit 语义，并同时保持 front/back 选择只影响 color readback。
- 当 Vulkan standalone 被构建时，应覆盖默认 framebuffer front/back color readback 的分离语义，确保 front 保留上一份已完成结果，而 back 跟随当前默认 framebuffer 写入。
- 当 Vulkan standalone 被构建时，应覆盖默认 framebuffer front/back color 作为 blit source 的分离语义，确保 `BlitFramebufferDesc.read_buffer` 不会在 default source 路径上退化失效。
- 当 Vulkan standalone 被构建时，应覆盖默认 framebuffer front/back color 作为 blit destination 的分离语义，并至少覆盖一条 default-to-default color blit 路径。
- 当 Vulkan standalone 被构建时，应覆盖 default-to-default depth/stencil blit 路径，并覆盖至少一条 overlapping self-blit 回归。
- 当 Vulkan standalone 被构建时，应覆盖 color overlapping self-blit 的基本语义，包括 default framebuffer 与单个 attachment 的原地偏移复制。
- 当 Vulkan standalone 被构建时，应覆盖至少一种跨格式 color attachment blit，用于验证 destination 格式契约保持稳定。
- 当 Vulkan standalone 被构建时，应覆盖 `BlitFramebufferDesc.size` 在 default framebuffer 双向路径上的有效区域语义。
- 当 Vulkan standalone 被构建时，应覆盖 `BlitFramebufferDesc.size` / `draw_size` 在 attachment 与 default framebuffer 路径上的缩放 blit 语义，并至少包含一条 `FilterMode::kLinear` color scaled blit 回归。
- 当 Vulkan standalone 被构建时，应覆盖 destination rect 部分越界时的 clipped scaled blit 语义，至少包括一条 attachment 路径和一条 default framebuffer source 路径。
- 当 Vulkan standalone 被构建时，应覆盖负 `draw_origin` 与负 `read_origin` 的 clipped blit 语义，至少包括一条 attachment 路径和一条 default framebuffer source 路径。
- 当 Vulkan standalone 被构建时，应覆盖 default framebuffer 尚未 materialize 时、带负 `draw_origin` 的 attachment-to-default blit 路径。
