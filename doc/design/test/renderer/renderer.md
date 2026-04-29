# test/renderer 设计

## 1. 组成
- `renderer_logic_test.cc`
- `renderer_shader_test.cc`
- `renderer_state_test.cc`
- `renderer_buffer_test.cc`
- `renderer_mesh_test.cc`

## 2. 设计要点
- 目录围绕 renderer 的主要公共面分测试文件，而不是按被测类逐个碎片化。
- 多数测试会通过 `ScopedFakeDevice` 或 `ScopedFakeRendererDevice` 注入可观测后端替身。
- `renderer_state_test` 负责守住 `ShaderProgramBindings`、descriptor 化的 `ShaderProgram` 创建、framebuffer/readback/blit，以及 compute dispatch wrapper 这类“提交语义是否被正确下发”的回归，其中也包括 buffer binding 是否已收口进单次 `ApplyBindings()`、`ApplyBindings()` 是否会先显式激活 program、`DrawBindings()` 是否会把 bindings + draw 一次性下发，以及基于 `ComputeDispatchDesc` 的 `DispatchComputeBindings()` 是否把 bindings + dispatch 稳定收口成一次高层 compute 提交。
- 同一组 state 测试现在也负责守住 `RenderShader` / `ComputeShader` 不会再在构造阶段先独立提交 scene-common bindings，而是把 common bindings 合并进最终一次 draw/dispatch 提交。
- 同一组 state/buffer 测试也负责守住 `Device::ApplyBufferBindings()` 对 `SSBO` / `AutomicCounter` 初始化路径的承接语义，避免长期存活 wrapper 又回退成散落 `BindBase()`。
- `renderer_shader_test` 则补位守住 shader 文件解析与 loader 契约，包括 include 解析、配置路径，以及 `ShaderLoader` 是否先产出 `ShaderProgramDesc` 再进入 program 创建。
- 同一组 state 测试也负责守住共享 bindings helper 的拼装语义，避免 camera/model/frame-state 这些高频字段在不同 scene/shader 包装里重新分叉成不一致实现。
- 其中 render-object helper 的测试专门负责守住 draw 初始化里 `model + camera` 组合提交的统一模板，避免 renderer 与 playground scene 再次回到重复手拼。
- legacy `model/view/project/view_pos_ws` helper 的测试则负责守住矩阵式 shader wrapper 的旧约定不会在持续收口过程中失配。
- 同一组 state 测试也负责守住 `Framebuffer::BindScoped()` 这类 renderer 级作用域 helper，避免 scene/pass 再回到手工 `Bind()/Unbind()` 配对。
- 这组 framebuffer 生命周期测试也会继续守住 `ScopedBind::Reset()` 的早退语义，确保 render pass 结束与 framebuffer state 恢复能在作用域结束前立即发生。
- `renderer_state_test` 里的 framebuffer blit 用例同时负责守住默认整张 blit 语义，以及显式 `BlitOption` 到 `rhi::BlitFramebufferDesc` 的逐字段透传。
- framebuffer attachment helper 中 depth/stencil/type/format 的静态约束，也由 `renderer_state_test` 守住。
- `renderer_mesh_test` 则继续守住 mesh 的 GPU 上传与 draw 提交语义，其中包括 vertex attribute 上传是否已经通过 `VertexArrayBindingDesc + ApplyBinding()` 下沉到 RHI，以及 `BuildDrawDesc()` 是否能生成可复用的几何描述对象。
- `renderer_buffer_test` 则继续守住 `SSBO` / `TextureBuffer` / `AutomicCounter` 的提交语义，其中也包括 host 侧读回是否已经通过 `BufferReadDesc + ReadData()` 下沉到 RHI。
- `TextureRepo::AsTexture2DArray()` 的缓存引用测试也放在 `renderer_state_test`，用于守住 `ProgramBindings` 保存 texture 地址时不会拿到临时对象。
- `Scene::GetTexture()` / `Framebuffer::GetTexture()` 的稳定引用测试也放在 `renderer_state_test`，用于守住 repo/attachment 查找接口不会重新退回按值返回临时 `Texture`。
- texture readback descriptor 的 renderer 侧用例也放在 `renderer_state_test`，用于守住 `Texture::GetData()` 不会重新回退到裸 `level/size` 参数调用。
