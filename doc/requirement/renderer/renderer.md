# renderer 需求

## 1. 目标
`renderer/` 目录为场景层提供共享渲染能力，包括资源配置、shader program、mesh、texture、framebuffer、scene 基类和若干 GPU 数据结构。

## 2. 范围
- scene 基类与 inspector
- 配置解析与 repo
- mesh / object / material / primitive / light / shadow / BVH
- shader、texture、framebuffer、SSBO/atomic counter
- renderer 侧 OpenGL 辅助

## 3. 功能需求
- 应向场景层提供统一可复用的资源仓库与渲染组件。
- 应尽量通过 `rhi::Device` 访问 GPU 能力，而不是直接散落后端调用。
- 应支持从 `Config` 加载 shader、texture 和 model 配置。
- 应支持 program bindings、framebuffer、render state、buffer binding 等公共渲染语义。
- 当 shader 提交路径已经采用 `ShaderProgramBindings` / `ProgramBindings` 时，buffer binding 也应能在同一次 `ApplyBindings()` 中表达并下发。
- 当 shader 提交路径已经采用 `ShaderProgramBindings` / `ProgramBindings` 时，storage texture 绑定也应在同一次 `ApplyBindings()` 中表达并下发，而不是依赖额外的 device 级单独提交。
- `renderer::ShaderProgram` 的创建也应采用单一的 descriptor 入口，与底层 `rhi::ProgramDesc + CreateProgram()` 保持同构，而不是继续维持独立的 render/compute 两套构造模式。
- `renderer::ShaderLoader` 应优先先产出 `ShaderProgramDesc`，再由 `ShaderProgram` 消费该 descriptor 创建底层 program，而不是在加载阶段直接分叉成两套 program 构造流程。
- 当 compute shader 的资源状态在 dispatch 前已完整可知时，renderer 也应支持通过统一入口一次性提交 bindings 与 `ComputeDispatchDesc`，而不是把 `Use/ApplyBindings/Dispatch/Barrier` 再拆回业务层。
- 当一次 shader/program 提交已经具备完整 bindings 时，renderer 应优先直接通过 `ApplyBindings()` 完成这次显式提交，而不是在业务层重复手动拆分“先激活再提交”。
- `renderer::ShaderProgram::ApplyBindings()` 应负责 program 激活与 bindings 下发，而不是让调用方隐式依赖“这个 program 之前已经被激活过”。
- 当一次 render 提交同时已具备完整 bindings 与 draw 参数时，renderer 还应优先通过 `ShaderProgram::DrawBindings()` 一次性下发 bindings 与 `DrawDesc`，而不是重新回退成 `ApplyBindings()` 后再 `Submit()` 的两步调用。
- `renderer::ShaderProgram` 不应继续对外保留 `SetBool/SetFloat/SetInt/SetMat4/SetVec*/SetTexture` 这类 named setter 透传接口，也不应继续保留单独的 program 激活入口；高层 shader 提交应统一收敛到 `ShaderProgramBindings`、`ApplyBindings()` 与基于 `ComputeDispatchDesc` 的 compute dispatch 包装。
- `renderer::RenderShader` 与 `renderer::ComputeShader` 这类高层包装不应继续保留 `SetModel()`、`SetCamera()`、`SetMaterial()`、`SetWorkGroupNum()`、`Run()` 这类状态机式提交通道；render path 应通过共享 bindings helper + `ApplyBindings()` 收口，compute path 应通过共享 helper + `ComputeDispatchDesc` 收口。
- `RenderShader` / `ComputeShader` 若需要 scene-common repo/buffer/texture bindings，也应把这部分 common bindings 合并进最终一次 draw/dispatch 提交，而不是在 wrapper 构造阶段先独立提交一次。
- 对不直接触发 draw、而只是补齐 render program 资源状态的 wrapper，renderer 也应继续通过 `RenderShader` 自身的合并型 `ApplyBindings()` 提交 common bindings + 局部 bindings，而不是绕回裸 `ShaderProgram`。
- 对 fullscreen-quad、debug overlay 这类“构造后立刻提交”的 render wrapper，也应优先直接走 `DrawBindings()` 或等价的一次性 draw helper，而不是停在 `ApplyBindings()` 后再依赖外部补 draw。
- 对 camera、model、material index、primitive start index、resolution、frame num、dirty 这类高频提交项，renderer 应提供可复用的 bindings helper，使 scene / shader 包装层能把它们并入一次 batched `ShaderProgramBindings` 提交。
- 对 render path 中最常见的 `model + camera` 初始提交，renderer 还应提供组合型 helper，避免各 scene/shader 再次重复手写两段绑定拼装。
- 对仍使用 legacy `model/view/project` 与 `view_pos_ws` uniform 约定的 render shader，renderer 也应提供对应 helper，并继续通过 batched bindings 一次性兑现这组旧约定。
- `mesh.*` 的 vertex attribute 上传也应优先通过 `rhi::VertexArrayBindingDesc + VertexArray::ApplyBinding()` 收口，而不是在 renderer 业务层直接拼装 `EnableAttribute()`、`SetFloatAttribute()`、`SetAttributeDivisor()`。
- `mesh.*` 还应支持先物化可复用的 `rhi::DrawDesc`，再由 `Submit()` 或 `ShaderProgram::DrawBindings()` 消费，避免 draw 几何描述被绑定在单一提交路径里。
- `SSBO`、`AutomicCounter` 这类长期存活 wrapper 的 buffer base 初始化，也应优先通过 `rhi::Device::ApplyBufferBindings()` 完成，而不是继续在 renderer 业务层直接调用 `BindBase()`。
- 对 renderer 而言，`BindBase()` 应只剩底层对象原语含义；wrapper 的正式初始化 contract 应统一落在 `ApplyBufferBindings()`，避免业务代码和测试把两条语义混在一起。
- inspector、debug overlay 这类需要临时关闭 depth/cull 的 renderer 辅助路径，也应优先通过 `rhi::ScopedRenderState` 收口状态恢复，而不是继续手工写成关闭/再打开的状态切换。
- `SSBO` 这类需要 host 侧读回的 wrapper 也应优先通过 `rhi::BufferReadDesc + Buffer::ReadData()` 收口，而不是继续依赖裸 `Map()/Unmap()`。
- `texture.*` 的 GPU 侧 host readback 也应优先通过 `rhi::TextureReadDesc + Device::ReadTextureData()` 收口，而不是继续让 renderer 业务层重复传裸 `level/size` 参数。
- `TextureRepo::AsTexture2DArray()` 应返回 repo 缓存的稳定 `Texture` 引用，确保把结果放进 `ShaderProgramBindings` 后不会出现悬垂 texture 指针。
- `Scene::GetTexture()` 与 `Framebuffer::GetTexture()` 也应作为纯查找接口返回稳定的 owned `Texture` 引用，而不是按值拷贝或继续保留没有实际兑现能力的 reload 变体，避免 scene/pass wrapper 把临时 texture 地址挂进 `ShaderProgramBindings`。
- `framebuffer.*` 应通过 `rhi::FramebufferDesc` 配置离屏 attachment，而不是在 renderer 层拼接后端 attachment 调用。
- `framebuffer.*` 还应向 scene/pass 层提供 scoped bind helper，让高层优先依赖对象作用域而不是手工维护 `Bind()/Unbind()` 配对；`BindScoped()` 应是正式业务接口，而 `Bind()/Unbind()` 退回 renderer 内部实现细节。
- `Framebuffer::ScopedBind` 还应支持显式 reset，使高层在作用域结束前也能提前结束离屏 pass 并立即恢复 framebuffer state。
- `framebuffer.*` 的 blit 路径应通过 `rhi::BlitFramebufferDesc` 表达 source/destination framebuffer、attachment、origin、size 与 filter 选择。
- `framebuffer_attachment.*` 中的 stencil 定义应保持为独立 stencil attachment 与匹配纹理格式，而不是复用 depth 占位。

## 4. 子目录职责
- `mesh/`：基础网格构造与网格工具
- `proto/`：资源配置 protobuf 模型
- `shader/`：通用 GLSL 与 shader 库

## 5. 验收要点
- `test/renderer` 与 `playground/test/renderer_test.cc` 可覆盖本目录关键行为。
