# rhi 需求

## 1. 目标
`rhi/` 目录定义本工程的渲染硬件接口抽象，并提供后端实现装配点。

## 2. 功能需求
- 应定义统一的 `Device`、`Buffer`、`VertexArray`、`Program`、`Capabilities` 等抽象。
- 应支持查询运行时 backend 与 scene backend。
- 应对上层暴露 `FramebufferDesc`、`ReadPixelsDesc`、`BlitFramebufferDesc`、render pass、render state、buffer binding、texture binding、readback 等统一语义。
- 公共 render state 应通过 `RenderStateDesc + ApplyRenderState()` 统一提交，而不是继续暴露上层常规使用的逐项 depth/cull/front-face setter。
- 当调用点只需要在局部作用域里临时覆盖 render state 时，也应通过 `CaptureRenderState()` / `RestoreRenderState()` 或 `ScopedRenderState` 收口，而不是继续要求业务层手工成对恢复状态。
- viewport 应通过 `RenderPassDesc` 或 `FramebufferState` 参与提交与恢复，而不是继续暴露单独的公有 viewport setter。
- render pass 生命周期除了显式 `BeginRenderPass()/EndRenderPass()` 外，还应支持 scoped helper 形式的高层收口，避免调用点手工维护 begin/end 配对。
- framebuffer state 生命周期除了显式 `CaptureFramebufferState()/RestoreFramebufferState()` 外，也应支持 scoped helper 形式的高层收口，避免调用点手工保存和恢复 state 对象。
- 应通过 `ClearDesc` 一次性表达非 attachment-selective clear 的 mask、color、depth clear value 与 stencil clear value，而不是把 clear color 拆成额外的隐式设备状态。
- `VertexArray` 的常规 attribute 配置应优先通过 `VertexArrayBindingDesc + ApplyBinding()` 一次性提交，而不是继续要求上层显式拼接 `EnableAttribute()`、`SetFloatAttribute()`、`SetAttributeDivisor()`。
- 对应的逐项 attribute setter 也不应继续作为 `VertexArray` 的上层常规公有接口。
- 需要独立于 program 生命周期存在的 buffer base 绑定，也应优先通过 `Device::ApplyBufferBindings()` 这类显式 descriptor 入口提交，而不是让业务层散落直接 `BindBase()`。
- `BindBase()` 本身可继续作为底层对象原语存在，但不应与 `ApplyBufferBindings()` 的显式 descriptor contract 混为同一层级语义。
- buffer 的 host 侧读回应优先通过 `BufferReadDesc + ReadData()` 显式表达 offset/size，而不是继续暴露上层常规使用的 `Map()/Unmap()`。
- texture 的 host 侧读回应优先通过 `TextureReadDesc + ReadTextureData()` 显式表达 level/size，而不是继续暴露裸参数式读回调用。
- `Program` 对上层暴露的常规提交面应收敛到单一的 `ApplyBindings()`；named uniform/texture setter、独立激活接口和 program id 不应继续作为上层常规依赖。
- 当调用方已经同时掌握 `ProgramBindings + DrawDesc` 或 `ProgramBindings + ComputeDispatchDesc` 时，RHI 也应提供一次性 convenience 提交，避免这类语义只散落在更高层包装中。
- `RenderPassDesc.viewport_size` 应表达正尺寸 viewport，而不是允许零或负的 render pass 宽高。
- render pass 驱动的 attachment-selective color clear 应只命中当前 draw framebuffer 上真实存在的 color attachment；默认 framebuffer 只允许 attachment 0。
- `ReadPixelsDesc` 在 attachment 选择上也应遵守同一原则：默认 framebuffer 只允许 attachment 0，非默认 framebuffer 则只允许真实挂载的 attachment。
- `BlitFramebufferDesc` 应能同时表达 source rect 与 destination rect，而不是只允许同尺寸 copy。
- `BlitFramebufferDesc` 引用的 source / destination attachment 也应遵守同一原则：默认 framebuffer 只允许 attachment 0，非默认 framebuffer 则只允许真实挂载的 attachment。
- `BlitFramebufferDesc` 的 `read_origin` / `draw_origin` 允许为负，此时后端应按部分越界 rect 处理，而不是直接失败。
- 当 `BlitFramebufferDesc` 的 source rect 或 destination rect 部分超出边界时，后端应保持两侧 rect 的映射关系并同步裁剪有效区域。
- `ReadPixelsDesc` 应能同时表达默认 framebuffer 的 front/back color 读回，以及默认/用户 framebuffer 的显式 attachment type / attachment index 读回。
- 应对 `DrawDesc + Draw()` 约束最基本的图形提交前置状态，至少包括 active render pass、active program、可用的 vertex array，以及 `DrawDesc.kind == kElements` 时可用的 index buffer。
- `DrawDesc` 应显式携带本次 draw 要使用的 `vertex_array`，并在 `DrawKind::kElements` 时显式携带 `index_buffer`，以彻底去除上层对独立 `Bind()` 状态机的依赖。
- 上层若需要先准备 draw 几何参数、再与 program bindings 组合提交，RHI 侧的 `DrawDesc` 也应保持可复用、可转交，而不是要求 draw 参数只能在最终 `Draw()` 调用点临时拼装。
- 应对 `DispatchCompute` 约束最基本的 compute 提交前置状态，至少包括 active compute program、非 render pass 环境和非零 workgroup。
- 应允许 OpenGL 与 Vulkan 后端在同一抽象下运行。

## 3. 子目录职责
- `gl/`：OpenGL 后端
- `vulkan/`：Vulkan 后端与 presenter

## 4. 验收要点
- `test/rhi` 应覆盖本目录核心行为。
