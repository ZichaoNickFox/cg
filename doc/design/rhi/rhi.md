# rhi 设计

## 1. 角色
`rhi/` 是 renderer 与具体图形后端之间的抽象边界。

## 2. 主要文件
- `types.h`：基础枚举和渲染类型
- `device.h/.cc`：抽象接口与全局活动设备管理
- `gl/`：OpenGL 实现
- `vulkan/`：Vulkan 实现

## 3. 设计要点
- 当前采用“全局单活动设备”模型。
- `Capabilities` 同时服务设备自描述与场景准入门禁。
- framebuffer 初始化已经从逐步 attachment 调用收敛到 `FramebufferDesc + ConfigureFramebuffer()`。
- 公共 render state 也已经收口到 `RenderStateDesc + ApplyRenderState()`；逐项 depth/cull/front-face setter 不再作为 `Device` 的公有提交面。
- 在这条路径上，render state capture/restore 也已经补齐成正式原语，并向上提供 `ScopedRenderState`，让临时状态覆盖不再手工写成成对的关闭/恢复调用。
- viewport 也已经从单独 setter 收口到 `RenderPassDesc` 与 `FramebufferState`；后端内部仍可保留 helper，但不再作为 `Device` 的公有常规接口暴露。
- render pass 生命周期本身也开始提供 scoped helper：它不改变 backend 的 begin/end 语义，但允许上层把 begin/end 配对关系收口进对象生命周期。
- framebuffer state 生命周期也开始提供 scoped helper：它不改变底层 capture/restore 原语，但允许上层把保存/恢复配对关系收口进对象生命周期。
- 非 attachment-selective clear 也已经从 `SetClearColor() + Clear(mask)` 这类拆散状态收敛到 `ClearDesc + Clear()`。
- `VertexArray` 的常规 attribute 配置路径也开始从手工 `Bind()/EnableAttribute()/SetFloatAttribute()/SetAttributeDivisor()` 收口到 `VertexArrayBindingDesc + ApplyBinding()`，让 backend 自己兑现一次 attribute/buffer 配置。
- 与此相应，那组三粒度 attribute setter 已不再保留在 `VertexArray` 的上层公有提交面，只作为 backend/fake 的内部实现细节存在。
- 与这条“descriptor 化 buffer 语义”的方向一致，需要独立于 program 生命周期存在的 buffer base 绑定，也开始通过 `Device::ApplyBufferBindings()` 收口，而不是让上层继续散落 `BindBase()`。
- `BindBase()` 因此更多退回成底层对象原语；descriptor 级的 buffer-binding 统计和测试回归则只围绕 `ApplyBufferBindings()` 建立。
- buffer 的 host 侧读回路径也开始从裸 `Map()/Unmap()` 收口到 `BufferReadDesc + ReadData()`；backend 仍可保留内部映射 helper，但不再把裸映射暴露成上层常规接口。
- texture 的 host 侧读回路径也开始从裸 `ReadTextureData(texture, level, data, size)` 收口到 `TextureReadDesc + ReadTextureData()`，让 level/size 契约和其它 readback descriptor 保持同一风格。
- storage texture 绑定也已经从 `Device::BindStorageTexture()` 这类单独的公有提交通道收回到 `ProgramBindings` / `Program::ApplyBindings()`。
- `Program` 的公有提交面也已经收口到单一的 `ApplyBindings()`；named uniform/texture setter、独立激活入口和 program id 不再继续作为上层常规接口暴露。
- 在这条基础上，RHI 现在也开始直接提供 `ProgramBindings + DrawDesc`、`ProgramBindings + ComputeDispatchDesc` 的 convenience 提交，让“一次激活并 draw/dispatch”的语义可以停留在抽象层，而不是只靠 renderer 自己拼接。
- `BeginRenderPass()` 现在也开始把 viewport 当成显式契约的一部分：render pass 的 viewport size 需要是正尺寸，而不是把零/负宽高继续透传给底层。
- render pass 内 attachment-selective color clear 的语义也开始从“尽量执行”收紧成“必须命中当前 draw framebuffer 上合法存在的 color attachment”。
- framebuffer/default-buffer 的像素读回已经从显式切换 read buffer 的方式收敛到 `ReadPixelsDesc`，并且 readback 目标已经从“默认 color attachment”扩展到显式的 attachment type / attachment index；其中 `ReadBuffer` 选择只对默认 color 生效，同时非法 attachment 也不再静默退化成空读回。
- framebuffer blit 已经从位置参数式接口收敛到 `BlitFramebufferDesc`，并开始显式表达 source rect 与 destination rect。
- `BlitFramebufferDesc` 的 source / destination attachment 合法性也开始被显式守门，避免非法 attachment index 再静默退化成 no-op。
- `BlitFramebufferDesc` 的 origin 语义也已经从“只接受非负坐标”推进到“负 origin 走部分越界裁剪”，更接近真实图形后端的 rect 处理方式。
- `BlitFramebufferDesc` 的实现语义不仅包括 offset/size，也包括 source / destination rect 部分越界时的同步裁剪；backend 不能把完整 source 直接缩到被截断后的 destination。
- `RenderPassDesc` 已补齐 depth/stencil clear value，不再把这两个 clear 值写死在 backend 内部。
- standalone draw 现在也开始显式守住 graphics submission 的前置状态：活动 render pass、活动 program、`DrawDesc` 显式提供的 vertex array，以及 `DrawDesc.kind == kElements` 时显式提供的 index buffer。
- 这也让 `DrawDesc` 能稳定作为高层 render wrapper 与 mesh 之间的交接对象存在，而不必把 geometry binding 重新拆回独立的 `Bind()` 状态机。
- compute dispatch 也开始显式守住前置状态，并且 RHI 公有面已经把 `DispatchCompute + MemoryBarrier` 收口成单次 `ComputeDispatchDesc`；renderer 侧也已通过基于 `ComputeDispatchDesc` 的 `ShaderProgram::DispatchCompute()` 和 `ShaderProgram::DispatchComputeBindings()` 继续收口 program 激活、bindings 提交与 dispatch。
- 抽象已逐步统一，但仍保留部分显式状态机式接口，属于演进中的兼容层。
