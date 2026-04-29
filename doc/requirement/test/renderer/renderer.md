# test/renderer 需求

## 1. 目标
本目录验证 renderer 层的重要契约。

## 2. 当前测试目标
- `renderer_logic_test`
- `renderer_shader_test`
- `renderer_state_test`
- `renderer_buffer_test`
- `renderer_mesh_test`

## 3. 功能需求
- 应覆盖 renderer logic、shader、state、buffer、mesh 等主题。
- 对需要设备观测的测试，应优先复用 fake device。
- renderer 测试应重点关注“提交给 RHI 的语义是否正确”，而不是依赖真实窗口输出。
- renderer state 测试应覆盖 compute shader/program 对 `ComputeDispatchDesc` 的提交语义，以及 `DispatchComputeBindings()` 这类高层统一入口是否正确下发 bindings 与 dispatch。
- renderer state 测试还应覆盖 buffer binding 是否能通过单次 `ApplyBindings()` 正确下发。
- renderer state 测试还应覆盖 `ShaderProgram::ApplyBindings()` 是否会先显式激活 program，再下发 bindings，避免重新退回对隐式活动 program 的依赖。
- renderer state 测试还应覆盖 `ShaderProgram::DrawBindings()` 是否会把 bindings 与 `DrawDesc` 一次性正确下发。
- renderer state 测试还应覆盖 `ShaderProgram` 创建是否已经收口成 descriptor，并正确把 render / compute program 元信息下发到 RHI。
- renderer state 测试还应覆盖 batched bindings 合并后是否仍能通过单次 `ApplyBindings()` 完成完整提交，避免重新退回手工拆分“激活 + bindings 提交”。
- renderer state 测试还应覆盖 `RenderShader` / `ComputeShader` 的 scene-common bindings 是否已收口进最终一次 draw/dispatch 提交，而不是重新退回构造阶段的独立预提交。
- renderer state / buffer 测试还应覆盖 `Device::ApplyBufferBindings()` 是否正确承接 `SSBO`、`AutomicCounter` 这类长期存活 wrapper 的一次性 buffer base 初始化。
- renderer shader 测试还应覆盖 `ShaderLoader` 是否已先构造 `ShaderProgramDesc`，并正确区分 render / compute shader 的 stage 集合。
- renderer state 测试还应覆盖共享 bindings helper 是否能把 camera/model/resolution/frame-state 正确拼入一次提交。
- renderer state 测试还应覆盖组合型 render-object helper 是否能把 `model + camera` 正确拼入一次提交。
- renderer state 测试还应覆盖 legacy `model/view/project/view_pos_ws` helper 是否仍能正确拼入一次提交，避免 legacy matrix shader wrapper 退回手写 uniform。
- renderer state 测试还应覆盖 framebuffer attachment helper 对 stencil type/format 的定义正确性。
- renderer state 测试还应覆盖 `Framebuffer::BindScoped()` 是否会把 bind/unbind 生命周期正确收口到对象作用域。
- renderer state 测试还应覆盖 `Framebuffer::ScopedBind::Reset()` 是否会在作用域结束前提前结束 render pass 并立即恢复 framebuffer state。
- renderer state 测试还应覆盖 `Framebuffer::Blit()` 对显式 blit option 的 RHI 透传语义。
- renderer state 测试还应覆盖 `TextureRepo::AsTexture2DArray()` 是否返回稳定缓存引用，避免把临时 `Texture` 地址塞进 bindings。
- renderer state 测试还应覆盖 `Scene::GetTexture()` 与 `Framebuffer::GetTexture()` 是否返回稳定 owned 引用，避免 scene/pass wrapper 再把按值拷贝出来的临时 `Texture` 地址塞进 bindings。
- renderer mesh 测试还应覆盖 vertex attribute 上传是否已经收口为 `VertexArrayBindingDesc + ApplyBinding()`，避免回退成 renderer 层手工 attribute setter。
- renderer mesh 测试还应覆盖 `Mesh::BuildDrawDesc()` 是否能在不触发 draw 的前提下准备可复用的几何提交描述。
- renderer buffer 测试还应覆盖 `SSBO` 的 host 侧读回是否已经收口为 `BufferReadDesc + ReadData()`，避免回退成 renderer 层裸 `Map()/Unmap()`。
- renderer state 测试还应覆盖 texture GPU readback 是否已经收口为 `TextureReadDesc + ReadTextureData()`，避免 renderer 重新回退到裸 `level/size` 参数调用。
