# playground/renderer_scene 需求

## 1. 目标
本目录承载渲染实验场景源码，是 OpenGL/Vulkan scene 路径最核心的业务场景集合。

## 2. 当前正式纳入构建与场景目录的内容
- `ModelScene`
- `ShadowScene`
- `SampleScene`
- `GeometryScene`
- `RayTracingScene`
- `PathTracingGeometryScene`
- `PathTracingScene`
- `RTRTScene`
- `SSAOScene`

## 3. 功能需求
- 应为上述场景提供可实例化的 `cg::Scene` 子类实现。
- 目录中的场景应尽量复用 `renderer/` 的公共 repo、shader program 与 framebuffer 能力。
- 需要 compute、storage buffer 或更高 shader 能力的场景，应在上层通过 capability 门禁控制准入。
- 当场景在构造 shader/compute pass 时已经掌握完整的初始 camera、model、buffer、storage texture、resolution 等提交状态，应优先把这些状态合并进单次 `ShaderProgramBindings` / `ApplyBindings()` 提交；对 compute 路径，还应优先通过一次性 dispatch helper 把 bindings 与 workgroup 一起提交，而不是散落成多次小的 setter 调用、`SetWorkGroupNum() + Run()` 两段式调用，或依赖“上一处刚刚激活过”的隐式前提。
- 对 geometry/shadow 等典型 draw pass，场景应优先复用 renderer 提供的 render-object helper 来提交 `model + camera`，而不是在每个 scene 里重复拼接这组基础状态。
- 场景侧若复用 `renderer/shader.*` 自带 wrapper，也不应再依赖 `RenderShader::SetModel()`、`SetCamera()`、`SetMaterial()` 这类历史 setter，而应直接复用共享 bindings helper 与一次性提交入口；对“构造后立刻 draw”的 wrapper，应优先走 `DrawBindings()`，而不是重新拆回 `ApplyBindings()` + `Run()/Submit()`。
- 对离屏 geometry/shadow/raster pass，场景也应优先复用 `Framebuffer::BindScoped()` 这类作用域入口，而不是继续手工维护 `Bind()/Unbind()` 配对。
- 对 cubemap/BRDF 生成工具这类短生命周期离屏 pass，也应遵守同一原则，优先通过 scoped framebuffer / render state helper 收口局部状态恢复。
- 继承 `renderer::Scene` 的场景还应默认复用 `Scene::Enter()` 设定的基础 render state；如果只是继续使用默认 `depth_test_enabled = true`，不应在各自 `OnEnter()` 里重复声明同一状态。
- 当前未被加入 `CMakeLists.txt` 或未被 `Playground` 注册的场景/工具源码，不应视为正式交付能力。

## 4. 验收要点
- 已注册场景可被 `Playground` 场景选择器识别。
- 目录中正式构建的场景与其对应 shader、resource 配置能对齐。
