# test/rhi 设计

## 1. 组成
- `rhi_logic_test.cc`
- `vulkan_device_test.cc`

## 2. 设计要点
- `rhi_logic_test` 更关注抽象层行为。
- `rhi_logic_test` 里也会放 `ClearDesc` 这类显式描述接口的 forwarding/capture 用例，用于守住 RHI 公共契约不会重新退回隐式状态。
- `VertexArrayBindingDesc` 这类配置型 descriptor 的 forwarding/capture 用例也放在 `rhi_logic_test`，用于守住 mesh/renderer 不会重新回退成手工 `EnableAttribute()/SetFloatAttribute()/SetAttributeDivisor()`。
- `Device::ApplyBufferBindings()` 这类独立 buffer binding descriptor 的 forwarding/capture 用例也放在 `rhi_logic_test`，用于守住长期存活 wrapper 不会重新回退成散落 `BindBase()`。
- 同一组逻辑测试也会显式区分 direct `BindBase()` 与 `ApplyBufferBindings()`，确保 fake/test 里的“descriptor 提交统计”不再把两条语义混在一起。
- `BufferReadDesc` 这类 host readback descriptor 的 forwarding/capture 用例也放在 `rhi_logic_test`，用于守住上层不会重新退回裸 `Map()/Unmap()`。
- scoped render state 的 capture/restore 用例也放在 `rhi_logic_test`，用于守住临时 depth/cull/front-face 覆盖不会重新退回手工状态恢复。
- `vulkan_device_test` 只在 Vulkan target 存在时参与构建和执行。
- 该目录同时覆盖“全局活动设备管理”和“具体后端语义”两类问题。
- `vulkan_device_test` 中的 standalone 用例除了资源/readback/blit 外，也负责守住 draw 状态机回归。
- 其中 draw 状态机回归应明确守住 `DrawDesc.vertex_array` 与 indexed path 的 `DrawDesc.index_buffer` 这两条显式几何提交契约，而不是继续允许测试依赖旧的预绑定几何状态。
- `vulkan_device_test` 里的 standalone 语义用例也开始覆盖 render state capture/restore，确保 synthetic scene backend 的 depth/cull/front-face 跟踪不会回退。
- viewport 合法性对 `BeginRenderPass()` 的守门用例也放在 `vulkan_device_test`，用于确保 zero/negative viewport size 不会进入 standalone render pass 生命周期。
- non-default framebuffer 完整性对 `BeginRenderPass()` 的守门用例也放在 `vulkan_device_test`，用于确保 standalone 不会在 incomplete FBO 上开启 pass。
- `RenderPassDesc.color_attachments` 合法性的守门用例也放在 `vulkan_device_test`，用于确保 default/non-default 两条路径都不会静默接受无效的 color attachment 声明。
- depth/stencil clear 附件存在性的守门用例也放在 `vulkan_device_test`，用于确保 non-default pass 不会在缺失 depth/stencil attachment 的情况下静默跳过 clear。
- `ReadPixels()` 合法性的守门用例也放在 `vulkan_device_test`，用于确保默认/非默认 framebuffer 的非法 attachment 不会再静默退化成全零读回。
- `BlitFramebuffer()` attachment 守门用例也放在 `vulkan_device_test`，用于确保默认/非默认 framebuffer 的非法 source/destination attachment 不会再静默退化成 no-op。
- `vulkan_device_test` 还负责守住 `ComputeDispatchDesc` 状态机回归，避免 standalone 重新退回为“任何时刻都能 dispatch，barrier 还得额外再打一遍”的旧行为。
- `ProgramBindings.buffers` 的 standalone 兑现用例也放在 `vulkan_device_test`，用于守住“单次 ApplyBindings 已覆盖 buffer binding”的回归。
- draw framebuffer 绑定对 clear/state capture 的影响也放在 `vulkan_device_test`，用于守住 synthetic backend 不再误写旧 framebuffer。
- split framebuffer state capture/restore 的用例也放在 `vulkan_device_test`，用于守住 read/draw 绑定不被 state round-trip 丢失。
- offset blit 的用例也放在 `vulkan_device_test`，用于守住 `BlitFramebufferDesc.read_origin` / `draw_origin` 不会在 standalone backend 中退化成固定左上角复制。
- stencil attachment 的 clear / blit 用例也放在 `vulkan_device_test`，用于守住 synthetic attachment 语义的完整性。
- attachment readback 用例也放在 `vulkan_device_test`，用于守住 `ReadPixelsDesc` 的 generic attachment 语义。
- color readback 格式转换用例也放在 `vulkan_device_test`，用于守住“按请求格式读回”而不是“只接受 exact payload 格式”的 standalone 行为。
- default framebuffer 的 depth/stencil clear、readback 与 blit 用例也放在 `vulkan_device_test`，用于守住 synthetic default attachment 语义，同时避免误把 `ReadBuffer` 选择扩展成非 color 附件概念。
- default framebuffer 的 front/back color readback 分离用例也放在 `vulkan_device_test`，用于守住 `ReadBuffer::kFront` / `kBack` 在 standalone synthetic 模式下不再退化成同一份颜色缓存。
- default framebuffer 的 front/back color source blit 用例也放在 `vulkan_device_test`，用于守住 `BlitFramebufferDesc.read_buffer` 在 default source 路径上能够真正区分 front/back。
- default framebuffer 的 front/back color destination 与 default-to-default blit 用例也放在 `vulkan_device_test`，用于守住 `BlitFramebufferDesc.draw_buffer` 在 default destination 路径上也能真正区分 front/back。
- default-to-default depth/stencil 与 overlapping self-blit 用例也放在 `vulkan_device_test`，用于守住非 color 默认附件路径不会在补齐 default-to-default 语义后又退回为原地覆写问题。
- overlapping self-blit 用例也放在 `vulkan_device_test`，用于守住 default color buffer 和单个 color attachment 在原地偏移复制时不会因为 source 被提前覆写而读错数据。
- 跨格式 color blit 用例也放在 `vulkan_device_test`，用于守住“destination attachment 元信息不被 source 覆写”的 synthetic blit 约束。
- partial default-to-attachment / attachment-to-default blit 用例也放在 `vulkan_device_test`，用于守住 `BlitFramebufferDesc.size` 的有效区域语义。
- scaled blit 用例也放在 `vulkan_device_test`，用于守住 `BlitFramebufferDesc.size` / `draw_size` 在 attachment、attachment-to-default 和 default-to-attachment 路径上的显式缩放语义，并同时覆盖 nearest 与 linear 两条 color filter 路径。
- clipped scaled blit 用例也放在 `vulkan_device_test`，用于守住 destination rect 部分越界时 source / destination 会按同一映射同步裁剪，而不是把完整 source 重新缩进被截断后的 destination。
- 负 origin 的 clipped blit 用例也放在 `vulkan_device_test`，用于守住 standalone backend 不会把负 `read_origin` / `draw_origin` 直接短路成空操作。
- lazy default framebuffer sizing 的负 origin 用例也放在 `vulkan_device_test`，用于守住 default framebuffer 尚未 materialize 时的最小 synthetic buffer 推导仍然遵守同一裁剪语义。
