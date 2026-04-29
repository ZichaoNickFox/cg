# renderer 设计

## 1. 角色
`renderer/` 是场景层和 RHI 之间的公共渲染中间层。

## 2. 主要组成
- 配置与加载：`config.*`、`model_loader.*`、`shader_loader.*`
- 资源与 repo：`mesh.*`、`object.*`、`texture.*`、`material.*`、`primitive.*`、`light.*`、`shadow.*`、`BVH.*`
- 运行时：`scene.*`、`io.*`、`camera.*`、`frame_stat.*`、`inspector.*`
- GPU 语义：`shader_program.*`、`framebuffer.*`、`ssbo.*`、`automic_counter.*`

## 3. 设计要点
- 目录通过 `cg_renderer` 暴露主要能力，通过 `cg_renderer_gl` 提供少量 OpenGL 辅助。
- `Scene` 基类把配置、repo、相机、inspector 和 capability 同步逻辑收口。
- `ShaderProgramBindings` 和 `ProgramBindings` 是 renderer 收敛 RHI 提交面的关键桥梁。
- `renderer::ShaderProgram` 的创建入口也已收口成单一 `ShaderProgramDesc`，并在内部直接映射到 `rhi::ProgramDesc + CreateProgram()`，避免 renderer 再单独维持 render/compute 两套 program 创建模式。
- `renderer::ShaderLoader` 也沿着同一条链路工作：它先解析 shader 文件并构造 `ShaderProgramDesc`，再由 `ShaderProgram` 消费该 descriptor，避免“文件加载阶段”和“program 创建阶段”各自维护一套 render/compute 分支判断。
- `ShaderProgram::ApplyBindings()` 现在依赖底层 `Program::ApplyBindings()` 一次性下发 uniforms/textures/storage textures/buffers，而不再单独补发 buffer-binding 调用。
- `ShaderProgram::ApplyBindings()` 会同时完成 program 激活与这批 bindings 下发，避免高层继续依赖“上一处刚好已激活”的隐式前提。
- `ShaderProgram::DrawBindings()` 则在这条基础上继续把 render 侧的 bindings 与 `DrawDesc` 收成一次高层提交，避免 scene/shader wrapper 重新拆回 `ApplyBindings()` + `Submit()`。
- 对不立刻触发 draw 的 render wrapper，`RenderShader` 也继续提供 wrapper 级 `ApplyBindings()`，用于在 common bindings 之上合并局部资源状态，而不必重新直接碰裸 `ShaderProgram`。
- 对 fullscreen-quad、debug overlay 这类构造后立刻完成 draw 的 render wrapper，这条收口进一步表现为直接在 wrapper 内部复用 `DrawBindings()`，不再停在“先 ApplyBindings，再由外部补 draw”的半收口状态。
- storage texture 也已经完全并入这条 `Program::ApplyBindings()` 路径，不再要求 renderer 侧保留额外的 device 级 storage image 提交步骤。
- `renderer::ShaderProgram` 也不再继续暴露 named uniform/texture setter 的透传面；renderer 侧真正对外保留的是 `ApplyBindings()`、`DrawBindings()`，以及基于 `ComputeDispatchDesc` 的 `DispatchCompute()` / `DispatchComputeBindings()` 这类提交式入口。
- `renderer::ShaderProgram` 也不再继续暴露 program id；program 身份只在 RHI/backend 内部用于活动状态追踪。
- 当 bindings 本身已经描述了一次完整 shader/program 资源提交时，renderer 应直接通过 `ShaderProgram::ApplyBindings()` 完成这次显式提交，并把 camera/material/common param 等高频提交尽量合并到更少次的 batched bindings 中。
- 为了让这类合并不只停留在个别 shader 类里，`renderer/shader.*` 进一步暴露了 camera/model/material index/primitive start index/resolution/frame num/dirty 的 append helpers，scene 和 shader 包装层可以直接复用这些 helper 来构建单次提交。
- 在此之上，render path 里反复出现的 `model + camera` 组合也被提炼成 render-object helper，而仍依赖 legacy `model/view/project/view_pos_ws` uniform 约定的 shader 则由另一组 helper 承接，让 renderer 自带 shader 包装和 playground scene shader 能共享统一的 bindings 构建模板，而不是重新退回到手写 named uniform。
- `RenderShader` / `ComputeShader` 这些更高一层的包装类也随之一起瘦身：render 侧不再暴露 `SetModel()`、`SetCamera()`、`SetMaterial()` 这类 setter，compute 侧也不再暴露 `SetWorkGroupNum()`、`Run()` 这类两段式入口，而是直接消费上述 helper、`ShaderProgramBindings` 和 `ComputeDispatchDesc`。scene-common repo/buffer/texture bindings 也会先缓存在 wrapper 内部，再合并进真正的单次 draw/dispatch 提交。
- `mesh.*` 的 GPU attribute 上传路径也开始一起收口：renderer 不再自己手工调用 `EnableAttribute()/SetFloatAttribute()/SetAttributeDivisor()`，而是构造 `rhi::VertexArrayBindingDesc` 后交给 `VertexArray::ApplyBinding()`，让 attribute/buffer 配置真正下沉到 backend。
- 同一方向上，`Mesh` 也开始把 draw 几何描述显式物化成 `rhi::DrawDesc`，`Submit()` 只是对 `BuildDrawDesc() + Draw()` 的便捷封装，这让 render wrapper 能直接把 mesh draw 参数接到 `ShaderProgram::DrawBindings()` 上。
- 这条“统一 buffer descriptor”路线也延伸到了长期存活 wrapper：`SSBO` / `AutomicCounter` 的一次性 buffer base 初始化不再直接散落 `BindBase()`，而是通过 `rhi::Device::ApplyBufferBindings()` 收口。
- 相应地，renderer 侧对“buffer-binding 是否被正确提交”的测试统计也只围绕 `ApplyBufferBindings()` 建立，而不再把 direct `BindBase()` 当成同一层级的 descriptor 提交。
- `SSBO` 的 host 侧读回路径也开始一起收口：renderer 不再从业务层直接调用裸 `Map()/Unmap()`，而是构造 `rhi::BufferReadDesc` 并通过 `Buffer::ReadData()` 完成显式读回。
- `Texture::GetData()` 的 GPU 侧读回路径也开始一起收口：renderer 不再把 `level/size` 当成裸参数散落传递，而是构造 `rhi::TextureReadDesc` 并通过 `Device::ReadTextureData()` 完成显式读回。
- `TextureRepo::AsTexture2DArray()` 也开始返回 repo 持有的缓存纹理引用，而不是临时值；这是因为 `ShaderProgramBindings` / `ProgramBindings` 当前保存的是 texture 地址，必须保证提交期内地址稳定。
- `Scene::GetTexture()` 与 `Framebuffer::GetTexture()` 也已经对齐到同一原则：高层只拿到稳定的 owned 纹理引用，不再拿到按值复制出来的临时 `Texture`；其中 `Scene::GetTexture()` 也已经简化成纯 repo 查找入口，不再携带无实际语义的 reload 变体。
- `framebuffer.*` 负责把 `FramebufferAttachment` 列表翻译成 `rhi::FramebufferDesc`，把 attachment 配置职责下沉到 backend。
- `framebuffer.*` 现在也开始复用 RHI 的 scoped render pass helper 来管理离屏 pass 生命周期，减少 renderer 层手工 begin/end 配对。
- `framebuffer.*` 也开始复用 scoped framebuffer state helper 来管理 bind/unbind 之间的状态恢复，减少 renderer 层手工保存和恢复 framebuffer state。
- 在此之上，`Framebuffer` 继续向上提供 `BindScoped()`，把 scene/pass 层最常见的 `Bind()/Unbind()` 配对也收回到 renderer 自己的对象作用域里；`Bind()/Unbind()` 本身退回 `Framebuffer` 内部生命周期细节。
- `Framebuffer::ScopedBind::Reset()` 则补齐了“提前结束当前离屏 pass”这条支路，确保作用域 helper 不会在早退路径上又逼业务层回退到裸 `Unbind()`。
- 对 inspector / debug overlay 这类只需要临时改动 depth/cull 的辅助渲染逻辑，renderer 现在也开始直接复用 RHI 的 `ScopedRenderState`，把 render state 恢复同样收回到作用域生命周期中。
- `framebuffer.*` 的 `Blit()` 负责构造 `rhi::BlitFramebufferDesc`；默认重载保留“整张 color attachment nearest blit”，而显式 `BlitOption` 重载负责把 attachment index、origin、source/destination size、mask 和 filter 继续透传给 RHI。
- `framebuffer_attachment.*` 现在明确把 stencil 描述成独立 attachment/type/format，避免 renderer 层把 stencil 退化成 depth 的别名。

## 4. 边界
- 具体设备实现不在本目录，而在 `rhi/`。
