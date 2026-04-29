# playground/renderer_scene 设计

## 1. 角色
本目录是实验场景的实现层，负责把 `renderer/` 的通用能力组织成具体渲染样例。

## 2. 当前构建与注册关系
- `CMakeLists.txt` 只把一部分场景编进 `cg_renderer_scene`。
- `Playground` 的 `GetSceneCatalog()` 再从已编译场景中挑出正式可选场景。
- 因此“目录里有源码”不等于“当前版本正式对外提供该场景”。

## 3. 设计要点
- 大部分场景继承 `cg::Scene` 并在 `OnEnter/OnUpdate/OnRender` 中组织自己的 shader、mesh、framebuffer 和 compute pass。
- 目录中同时保留了一些工具型或历史型场景源码，例如 cubemap/PBR 生成器、skybox、instance 等，作为后续扩展储备。
- 场景实现与私有 shader、resource 目录紧密耦合，但仍通过 `Config` 和 repo 获取最终资源。
- 随着 renderer 的 bindings 收口推进，场景侧的 shader/compute 包装也开始复用共享 append helpers，把 camera/model/buffer/storage texture/resolution/frame-state 等初始化提交合并到更少次的 `ApplyBindings()`、`DrawBindings()` / 基于 `ComputeDispatchDesc` 的 `DispatchComputeBindings()` 中。
- 对 render pass 来说，这种收口进一步表现为复用统一的 render-object helper，让 `model + camera` 不再在各个 scene shader 包装里重复写一遍。
- 对离屏 framebuffer pass 来说，这种收口也继续表现为优先复用 `Framebuffer::BindScoped()`，让 scene 代码不再手工维护 `Bind()/Unbind()` 生命周期。
- 对 inspector / tool scene 这类临时覆盖 depth/cull 或只跑一小段离屏 pass 的逻辑，也开始直接复用 scoped render state / framebuffer helper，把恢复职责留在局部生命周期里。
- 对基础 render state 来说，这种收口还表现为优先复用 `Scene::Enter()` 提供的默认 depth-test 基线；scene `OnEnter()` 只有在需要偏离默认值时才再显式调用 `ApplyRenderState()`。

## 4. 边界
- 不在此目录中定义运行时分流。
- 不在此目录中实现通用 RHI。
