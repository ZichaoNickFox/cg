# CG 设计说明

## 1. 文档目标
本文档对应 `doc/requirement/requirement.md`，回答以下问题：

- 这个工程当前由哪些模块组成，各自负责什么
- `run -> window -> Playground -> Scene -> Renderer -> RHI -> Backend` 这条链路如何工作
- OpenGL、Vulkan compat、Vulkan standalone 三条运行路径的真实差异是什么
- 当前设计已经稳定的部分和仍处于演进中的部分分别是什么

## 1.1 文档组织
本设计文档是总述。`doc/design/` 目录按一方源码和资产目录递归镜像，子目录中的局部文档用来描述各模块的职责、边界、主要文件和扩展方式。

这里的“镜像”不仅指覆盖同一批目录，也指目录层级和组织方式本身要与仓库根目录保持一致。也就是说，若根目录存在某个纳入文档范围的目录 `X/...`，则设计文档应在 `doc/design/X/.../` 下按相同层级组织；需求文档也应在 `doc/requirement/X/.../` 下按相同层级组织。

局部文档的文件名还应与其所在文件夹同名，而不是统一使用 `design.md` 或 `requirement.md`。也就是说：

- `doc/design/<dir>/<dir>.md`
- `doc/requirement/<dir>/<dir>.md`

仅两份总述文档保留：

- `doc/design/design.md`
- `doc/requirement/requirement.md`

例如：

- 根目录的 `renderer/shader/pbr/`，对应 `doc/design/renderer/shader/pbr/pbr.md`
- 根目录的 `test/test_support/`，对应 `doc/requirement/test/test_support/test_support.md`

镜像范围与需求文档保持一致，覆盖：

- `cmake`
- `Nsight`
- `md`
- `base`
- `geometry`
- `physics`
- `playground`
- `renderer`
- `rhi`
- `test`

及其递归子目录。

下列目录不在递归设计文档范围：

- `thirdparty`
- `build`
- `log`
- `.git`、`.vscode`

## 2. 设计目标与当前边界

### 2.1 设计目标
- 保持 OpenGL 路径持续可运行，作为场景正确性的稳定基线
- 让场景逻辑尽量通过统一的 renderer/RHI 抽象访问 GPU 能力
- 支持 Vulkan presenter 与 Vulkan scene backend 的渐进式接入
- 用自动化测试保护运行入口、资源绑定和后端演进行为
- 让 shader、texture、model 资源通过统一配置文件管理，而不是散落硬编码路径

### 2.2 当前边界
- 当前不追求所有场景在 `Vulkan standalone` 下与 OpenGL 完全等价
- 当前不追求把 `rhi::Device` 一次性重构成最终形态的现代显式图形 API
- 当前不追求引入完整编辑器、资源面板或运行时后端热切换

## 3. 源码结构与模块职责

当前工程的顶层模块大致如下：

```text
base/                  基础数学、工具和调试设施
geometry/              几何算法与绘图辅助
physics/               物理算法与实验支撑
renderer/proto/        protobuf 配置模型
renderer/              资源仓库、scene 基类、framebuffer、shader、texture 等
rhi/                   RHI 抽象和 OpenGL/Vulkan 后端
playground/            运行时入口、场景目录、窗口与实验场景
test/                  基础单元测试
playground/test/       renderer 与运行时 smoke tests
thirdparty/            第三方依赖
```

职责拆分如下：

- `base/`：提供 `util`、基础几何/数学与调试宏
- `renderer/proto/`：定义 `ShaderConfig`、`TextureConfig`、`ModelConfig`
- `renderer/`：承载配置读取、资源 repo、相机、场景基类、framebuffer 和 shader program 逻辑
- `rhi/`：定义统一的 `Device` 接口，并提供 `gl` 与 `vulkan` 后端
- `playground/`：组合场景和后端，驱动帧循环、UI 与运行时选择
- `test/` 与 `playground/test/`：分别验证静态逻辑与真实运行路径

## 4. 总体架构

系统的主链路如下：

```text
run / run.sh / 环境变量
  -> build/dev/bin/window
    -> runtime 选择
      -> Playground
        -> Scene
          -> renderer repos / helpers
            -> rhi::Device
              -> OpenGL backend
              -> Vulkan backend
```

如果把“展示窗口”和“场景渲染”进一步拆开，当前设计更准确地是：

```text
Presentation Runtime
  -> 可见窗口
  -> 最终屏幕展示

Scene Renderer
  -> 场景更新
  -> UI 绘制
  -> framebuffer / texture readback

Playground
  -> 把上面两者串起来
```

这也是为什么 `Presentation Runtime` 和 `Scene Renderer API` 在日志里是两个独立维度。

## 5. 启动与运行时分流设计

### 5.1 `run` 脚本
`run` 是统一入口，负责：

- 解析 `gl|opengl`
- 解析 `vk|vulkan [compat|standalone]`
- 设置 `CG_RUNTIME` 和 `CG_VK_SCENE_MODE`
- 规范代理环境变量
- 执行 `cmake --preset dev`
- 执行 `cmake --build --preset dev --target window`
- 启动 `build/dev/bin/window`

`run.sh` 只是它的轻量转发。

### 5.2 默认运行时
`playground/window.cc` 中的 `ChooseRuntimeMode()` 决定默认行为：

- 显式设置 `CG_RUNTIME` 时，按用户请求执行
- 未设置时，如果构建进了 Vulkan RHI，则默认走 Vulkan
- 否则默认走 OpenGL

这意味着 `./run` 本身就是一个有意义的模式选择入口，而不是“必须补参数”的占位脚本。

### 5.3 主程序入口
`main()` 的主流程是：

1. 初始化 glog
2. 初始化 GLFW
3. 读取窗口尺寸环境变量
4. 选择运行时
5. 进入某一个具体运行路径
6. 程序退出前销毁设备并关闭日志

### 5.4 运行路径矩阵

| 启动方式 | Presentation Runtime | Scene Renderer | Offscreen GL | 当前定位 |
| --- | --- | --- | --- | --- |
| `CG_GL_RUNTIME=native ./run gl` | OpenGL | OpenGL | 无 | 最稳定基线 |
| `./run gl`（Mesa 可用且未显式指定 `CG_GL_RUNTIME`） | OpenGL presenter | OpenGL | 有 | 默认 Mesa/llvmpipe 路径 |
| `CG_GL_RUNTIME=mesa ./run gl` | OpenGL presenter | OpenGL | 有 | 显式 Mesa/llvmpipe 路径 |
| `./run vk` | Vulkan presenter | OpenGL compatibility | 有 | 过渡态 |
| `./run vk standalone` | Vulkan presenter | Vulkan Synthetic RHI | 无 | 独立但实验性 |

## 6. 窗口、展示与帧循环设计

### 6.1 OpenGL native 路径
`RunLegacyOpenGL()` 直接：

- 创建可见 OpenGL 窗口
- 初始化 GL loader、ImGui、ImPlot
- 初始化 OpenGL RHI 设备
- 在同一个窗口中执行场景渲染与 UI
- 通过 `glfwSwapBuffers()` 展示结果

这是当前最接近传统即时渲染 playground 的路径。它会在以下情况下被选中：

- 显式设置 `CG_GL_RUNTIME=native`
- 请求 `CG_GL_RUNTIME=mesa` 但 Mesa runtime 不可用，系统回退到 native
- 构建本身未集成 Mesa offscreen runtime

### 6.2 Offscreen 渲染路径
`OffscreenGlRenderer` 是当前工程非常关键的桥接组件。它负责：

- 创建隐藏的 native OpenGL、OSMesa 或 Mesa EGL context
- 初始化场景渲染所需的 ImGui/OpenGL 环境
- 执行 `Playground` 的更新与渲染
- 从 framebuffer 读取 RGBA 像素
- 将像素帧返回给 presenter

这个设计让“场景渲染”和“最终展示”可以分离。

在当前实现里，只要构建集成了 Mesa offscreen runtime 且用户没有显式要求 `native`，OpenGL 运行时就会优先走这条路径。

### 6.3 OpenGL presenter
当场景在 offscreen OpenGL 中渲染，但最终仍需要 OpenGL 可见窗口展示时，程序会使用单独的 presenter 把 readback 结果画到屏幕上。这样 Mesa 路径可以做到：

- 逻辑上仍跑 OpenGL scene renderer
- 物理上不依赖把真实场景直接画到可见窗口

### 6.4 Vulkan presenter
`rhi/vulkan/presenter.*` 实现 Vulkan 展示层。它的职责不是参与场景逻辑，而是：

- 创建 Vulkan instance、device、swapchain 和 surface
- 接收一帧 RGBA 像素数据
- 上传到 Vulkan image
- 绘制全屏纹理并提交到 swapchain

所以在 `vk compat` 模式下，Vulkan presenter 只是“把别处渲染好的图显示出来”。

### 6.5 统一帧循环
无论具体运行路径如何，核心帧循环都遵循同一套结构：

1. 轮询窗口事件
2. 刷新 framebuffer 与 screen 尺寸
3. `Playground::BeginFrame()`
4. 为默认 framebuffer 构建 main render pass
5. 收集输入并写入 `Io`
6. `Playground::Update()`
7. `Playground::Render()`
8. 绘制 ImGui
9. 结束 render pass
10. swap 或通过 presenter 提交
11. `Playground::EndFrame()`

这里的关键变化是：默认 backbuffer 的 clear 和 viewport 已通过 `RenderPassDesc` 统一表达，而不是散写 OpenGL 状态调用。

## 7. Playground 与场景系统设计

### 7.1 Scene catalog
`playground/playground.cc` 使用 `SceneDescriptor` 维护显式注册的场景目录。每个 descriptor 当前包含：

- `id`
- `display_name`
- `description`
- `category`
- `requires_glsl_450`
- `requires_compute`
- `requires_storage_buffers`
- `create`

这套设计有三个好处：

- 当前交付能力是显式的，不依赖目录扫描
- 场景切换 UI 与启动时 `CG_SCENE` 校验使用同一份数据
- 场景能力门禁可以和目录定义绑定在一起

### 7.2 “文件存在”与“能力交付”分离
仓库中 `playground/renderer_scene/` 下存在比当前注册目录更多的源码文件，但不是每个文件都属于当前交付能力。是否纳入能力基线，取决于两件事：

- 是否被加入相应 `CMakeLists.txt`
- 是否在 `GetSceneCatalog()` 中注册

这个约束很重要，因为它避免了“代码历史遗留文件”被误认为当前用户可选功能。

### 7.3 场景分类
当前场景目录按以下分类展示：

- `Rendering`
- `Physics`
- `Animation`
- `Geometry`

其中 `Animation` 目前保留为扩展位，尚无注册场景。

### 7.4 初始场景和回退策略
`Playground` 构造时会：

- 初始化配置文件
- 读取 `CG_SCENE`
- 计算初始分类页
- 请求打开场景选择弹窗

`SwitchToScene()` 的回退策略是：

- 如果场景 ID 不存在，记录日志并在首次启动场景失败时回退到默认场景
- 如果场景能力不足，记录日志并在首次启动场景失败时回退到默认场景

### 7.5 场景生命周期
`renderer/scene.h` / `renderer/scene.cc` 定义统一生命周期：

- `Enter()`：绑定 `Config`、`Io`、`FrameStat`，初始化 repo、相机和默认 render state，再调用 `OnEnter()`
- `Update()`：更新相机 aspect、同步 GPU 公共数据、处理输入、调用 `OnUpdate()`、绘制 inspector
- `Render()`：调用 `OnRender()`
- `Exit()`：调用 `OnExit()`

这让各个实验场景只需要关注自己的渲染逻辑，而不必重复处理公共样板代码。

## 8. 输入、相机与 UI 设计

### 8.1 `Io` 作为输入聚合点
窗口层通过 `FillIoInput()` 将键盘、鼠标与 GUI 抢占状态写入 `Io`。`Scene` 不直接访问 GLFW，而是只读取 `Io`。

这有两个好处：

- 场景逻辑与窗口库解耦
- 输入行为更容易在测试或未来平台迁移时被替换

### 8.2 默认相机行为
`Scene::MoveCamera()` 提供统一相机控制：

- `W/S` 控制前后移动
- `A/D` 控制左右移动
- 鼠标左键拖动控制旋转
- `ESC` 直接退出程序

同时，当 `gui_captured_cursor()` 为真时，场景层停止处理鼠标旋转。

### 8.3 调试 UI
`Playground` 层维护两个主要 UI：

- `Scene Launcher`：当前场景、运行时信息、切换/重载入口
- `Select Scene` 弹窗：分类、场景、tooltip、能力状态和不支持原因

`Scene` 基类还会调用 `Inspector` 展示场景级调试信息。

## 9. 资源配置与内容加载设计

### 9.1 protobuf 配置模型
资源配置通过 `renderer/proto/config.proto` 定义：

- `ShaderConfig`
- `TextureConfig`
- `ModelConfig`
- `ConfigData`

设计选择 protobuf 的原因是：

- 配置结构明确
- 便于后续扩展字段
- 可以用文本格式直接纳入仓库审阅

### 9.2 默认配置文件
`playground/config.pb.txt` 是默认资源入口，集中声明：

- shader 名称与源码路径
- texture 名称、类型和资源路径
- model 名称与目录信息

### 9.3 配置加载流程
`Config::Init()` 的流程是：

1. 读取 `config.pb.txt`
2. 用 protobuf text format 解析为 `ConfigData`
3. 按类型拆分为 shader/texture/model 三类映射
4. 在加载过程中解析资源路径

### 9.4 路径解析策略
`ResolveConfigPath()` 当前按以下优先级解析资源路径：

1. 若传入路径已经是绝对路径，或当前工作区直接存在，则直接使用
2. 若相对项目根目录存在，则转成项目绝对路径
3. 若相对配置文件目录存在，则转成配置目录绝对路径
4. 否则保留原样返回

这个策略同时兼容：

- 已经写死的项目相对路径
- 相对配置文件的资源引用
- 本地实验时的绝对路径

## 10. Renderer 层设计

### 10.1 Scene 共享 repo
`Scene` 当前持有并复用以下 repo/资源集合：

- `ShaderProgramRepo`
- `TextureRepo`
- `MeshRepo`
- `ObjectRepo`
- `MaterialRepo`
- `PrimitiveRepo`
- `LightRepo`
- `ShadowRepo`
- `BVH`

这些对象承载了多数场景的公共数据，而不是让每个场景重复维护一套资源入口。

### 10.2 ProgramBindings
renderer 层正在把 shader 资源提交收敛到 `ProgramBindings`。当前这套模型已经可以承载：

- uniforms
- sampled textures
- storage textures
- buffers

它的设计目的，是用一次性提交代替散落的 `SetFloat()`、`SetInt()`、`BindBase()` 调用。当前 `buffers` 也已经进入同一次 `Program::ApplyBindings()` 提交，不再要求 `ShaderProgram` 在外层补发第二次专门的 device buffer-binding 调用。与此同时，`rhi::Program` 的公有提交面也已经进一步收敛到单一的 `ApplyBindings()`；named uniform/texture setter、独立 program 激活入口和 program id 都不再继续作为上层常规接口暴露。对应地，`renderer::ShaderProgram` 自身也不再继续保留 named uniform/texture setter 或单独的激活入口，而是把高层对外提交语义收敛到 `ApplyBindings()`、`DrawBindings()` 与基于 `ComputeDispatchDesc` 的 compute dispatch 包装；其中 `ShaderProgram::ApplyBindings()` 会同时承担 program 激活与 bindings 下发，`ShaderProgram::DrawBindings()` 则继续把 bindings 与 `DrawDesc` 收成一次 render 提交，避免 renderer/shader 包装层继续依赖“之前已经激活过”的隐式状态。renderer 侧的 program 创建也已进一步收口成单一 `ShaderProgramDesc`，并直接映射到 `rhi::ProgramDesc + CreateProgram()`；`ShaderLoader` 则先构造 `ShaderProgramDesc` 再交给 `ShaderProgram`，避免 renderer 在“shader 文件加载”和“program 创建”两段链路里各自维护一套 render / compute 分支。对于“这次 bindings 已经定义了完整 program 资源状态”的场景，renderer 直接复用 `ApplyBindings()` 作为更接近 submission 的统一入口；而当 bindings 与 draw 参数同时完整可知时，则优先复用 `DrawBindings()` 继续收口 render 提交。RHI 自身现在也开始提供 `ProgramBindings + DrawDesc`、`ProgramBindings + ComputeDispatchDesc` 的 convenience 提交，避免这类“一次激活并提交”的语义只留在 renderer 包装层。为减少 scene/shader 包装层重复拼 camera、model、resolution、frame state 这类高频数据，renderer 进一步提供了一组可复用的 bindings append helpers，让这些状态可以被合并进单次提交；其中 render path 最常见的 `model + camera` 也被继续收口成组合型 render-object helper，而 legacy `model/view/project/view_pos_ws` 约定则由独立 helper 承接，避免 wrapper 回退到手写 named uniform。
与这条收口方向一致，`RenderShader` / `ComputeShader` 这类高层包装也不再继续保留 `SetModel()`、`SetCamera()`、`SetMaterial()`、`SetWorkGroupNum()`、`Run()` 这类状态机式业务提交通道；render wrapper 直接构造 batched bindings，compute wrapper 则直接构造 `ComputeDispatchDesc` 并通过一次性 dispatch helper 下发。scene-common 的 repo/buffer/texture bindings 现在也不再在 wrapper 构造阶段先单独 `ApplyBindings()` 一次，而是先缓存到 wrapper 内部，再合并进真正的单次 draw/dispatch 提交。对于像 `PbrShader` 这类“不立即 draw、但需要补齐 render program 资源状态”的包装，`RenderShader` 也继续提供 wrapper 级 `ApplyBindings()`，用来在合并 common bindings 后完成同一套提交流程，而不是重新回退到直接操作底层 `ShaderProgram`。
同一方向上，storage texture 也不再要求上层保留单独的 `Device::BindStorageTexture()` 提交路径，而是跟 sampled texture、buffer 一样，优先作为 `ProgramBindings` 的一部分由 backend 在 `ApplyBindings()` 内兑现。

### 10.3 Buffer bindings
`SSBO`、`AutomicCounter` 和 repo 同步路径都在逐步改成统一的 `BufferBindingDesc` 机制。这样上层 scene/renderer 只表达“我要绑定什么”，不表达“底层用哪个 API 调哪个 bind 点”。当 buffer 绑定本来就属于某次 shader/program 资源提交时，这组 `BufferBindingDesc` 可以直接挂在 `ProgramBindings` 里，由 backend 在 `ApplyBindings()` 内完成兑现；而 `SSBO`、`AutomicCounter` 这类长期存活 wrapper 的一次性初始化，也已经进一步收口到 `Device::ApplyBufferBindings()`，避免业务层继续直接持有 `BindBase()` 风格调用。
与这条边界一致，`BindBase()` 现在更多只保留成 backend/对象内部原语；fake/test 侧对“显式 buffer-binding 提交”的统计也开始只追踪 `ApplyBufferBindings()` 语义，不再把任意 direct `BindBase()` 也混进同一条 descriptor contract。
与同一类“bindings 内保存 texture 地址”的约束对应，`TextureRepo::AsTexture2DArray()`、`Scene::GetTexture()` 和 `Framebuffer::GetTexture()` 现在都返回稳定的 owned 纹理引用，而不是临时值；其中 `Scene::GetTexture()` 也已经退回成纯 repo 查找接口，不再继续携带没有实际兑现能力的 reload 分支。

### 10.4 Framebuffer 与 render pass
renderer 层仍保留 `Framebuffer` 这样的业务封装，但离屏 framebuffer 初始化已经改成描述式配置：`renderer/framebuffer.cc` 先把 attachment 列表转换成 `FramebufferDesc`，再调用 `Device::ConfigureFramebuffer()`。pass 执行阶段则继续通过 `RenderPassDesc` 驱动。当前能统一表达的内容包括：

- 离屏 attachment 集合
- framebuffer 目标
- viewport 原点与尺寸
- color clear
- depth clear
- stencil clear
- depth/stencil clear value

这意味着高层已经不再自己拼接 `AttachFramebufferTexture2D()`、`SetDrawBuffers()` 之类的后端细节；这些动作收敛到 backend 内部执行。
在 standalone synthetic backend 中，split read/draw framebuffer 绑定已经被当作真正的写目标切换：后续 render pass 内的 attachment-selective color clear 和 framebuffer state capture 会跟随当前 draw framebuffer，而不是停留在旧的全局 framebuffer 概念上；这条语义通过 `FramebufferState` capture/restore 和 render pass 路径向上暴露，而不是继续保留单独的 `BindFramebuffer(...)` 公有 API。与此同时，`BeginRenderPass()` 会先要求 `RenderPassDesc.viewport_size` 为正尺寸，再继续进入 pass；在非默认 framebuffer 路径上，它还会再次验证目标 framebuffer 已存在且 complete，避免调用方忽略 `ConfigureFramebuffer(false)` 后仍把不完整 FBO 带进 draw 生命周期；`RenderPassDesc.color_attachments` 里声明的 attachment index 也会被显式校验，确保默认 framebuffer 只出现 attachment 0，而非默认 framebuffer 只引用真实挂载的 color attachment。对 `clear_depth` / `clear_stencil` 也是同一原则：non-default pass 只有在对应 depth/stencil attachment 真实存在时才允许打开 clear。
同一轮演进里，`FramebufferState` 也从“只记一个 framebuffer”收紧成“显式记 read/draw framebuffer，并把旧字段保留为 draw alias”，这样 `Framebuffer::Bind()/Unbind()` 这类 capture/restore 场景不会在 split binding 下丢状态。
renderer 自己的 `Framebuffer` 封装也继续沿着这条方向上收：业务层正式使用的是 `BindScoped()`，而 `Bind()/Unbind()` 已退回内部生命周期细节；`ScopedBind::Reset()` 则负责“在作用域结束前提前结束 pass 并立即恢复 framebuffer state”这条早退支路，避免 scene/pass 又回退到显式 `Unbind()`。

### 10.5 Readback
默认 framebuffer 和离屏 framebuffer 的像素读回，也开始从显式状态机迁向描述式接口：`playground/window.cc` 和测试不再手动先调用 `SetReadBuffer()`、再切 read framebuffer、再 `ReadPixels()`，而是直接提交 `ReadPixelsDesc`。这样 readback 的“读哪一个 framebuffer、读哪个 attachment type/index、默认 front/back 选哪边”都由一次调用表达；其中 front/back 选择只对 color 生效，但 standalone backend 现在也会 materialize 默认 framebuffer 的 depth/stencil 附件用于显式读回。为了让 `ReadBuffer::kFront` / `kBack` 在 synthetic 模式下也有实际语义，default color 现在拆成了独立的 front/back 缓冲：back 代表当前默认 framebuffer 的活动写目标，front 代表上一份已完成的默认 framebuffer 结果，并会在默认 render pass 结束时被刷新。对于常见 synthetic color 组合，standalone readback 也开始按 `ReadPixelsDesc` 的目标格式执行 CPU 侧 payload 转换，而不再要求 attachment payload 与请求格式完全一致。同时 readback 的 attachment 合法性也被收紧成正式守门：默认 framebuffer 只接受 attachment 0，非默认 framebuffer 则必须真的挂有对应 attachment，不能再静默回落成全零读回。

### 10.6 Blit
framebuffer blit 也开始脱离位置参数式接口。当前由 `BlitFramebufferDesc` 统一表达 source / destination framebuffer、default color source/destination 的 front/back 选择、color attachment 选择、read/draw origin、source size、destination size、mask 和 filter，这样 backend 不需要再假设“永远 blit color attachment 0”或“总是从左上角拷到左上角”。在 standalone synthetic backend 中，默认 framebuffer color 作为 source 时会跟随 `BlitFramebufferDesc.read_buffer` 选择 front/back，作为 destination 时会跟随 `BlitFramebufferDesc.draw_buffer` 选择 front/back，并补上 default-to-default 的 color copy 路径；depth/stencil 也已经补上 default-to-default 的 copy 路径，但不依赖 front/back 选择。`size/draw_size` 会共同决定 default framebuffer 与离屏 attachment 双向 copy 的真实区域；当 color path 的 source / destination size 不同时，standalone backend 也会显式做 synthetic 缩放，而不再把 blit 固定成“只支持同尺寸复制”。如果 `read_origin` 或 `draw_origin` 为负，standalone backend 会把它当成部分越界 rect 继续参与同一套缩放映射，而不是直接短路为空操作；即使 default framebuffer 还没有 materialize，lazy sizing 也会遵守同一套可见区域计算。如果 source rect 或 destination rect 部分超出边界，standalone backend 会按同一缩放映射同时裁剪 source / destination 的有效区域，而不是把完整 source 重新缩放进被截断后的 destination。与此同时，blit 目标 attachment 的合法性也开始 fail-fast：默认 framebuffer 路径只接受 attachment 0，非默认 framebuffer 则必须真实挂有被引用的 color/depth/stencil attachment。对于 default-to-default 或 attachment-to-same-attachment 这类 overlapping self-blit，standalone backend 现在会先快照 source 区域，再写回 destination，避免原地覆写污染后续 source 读取。

### 10.7 Render state
公共状态如 depth test、cull、front face 已通过 `RenderStateDesc` 和 `ApplyRenderState()` 收口。`Scene::Enter()` 会在通用入口中设置基础 render state。当前 `Device` 的公有接口也只保留这条描述式入口；单字段 setter 退回为 backend 内部 helper，不再作为上层常规 API。viewport 也不再通过单独公有 setter 调整，而是跟随 `RenderPassDesc` 或 `FramebufferState` 一起流动。
对只需要临时覆盖 render state 的上层逻辑，RHI 也开始提供 `CaptureRenderState()` / `RestoreRenderState()` 与 `ScopedRenderState`；这让 inspector、debug overlay 之类路径不再手工写成“先关闭 depth，再重新打开”的成对状态切换，而是把恢复责任收回到作用域生命周期里。

## 11. RHI 设计

### 11.1 抽象边界
`rhi/device.h` 是当前 RHI 的核心边界，统一定义：

- `Capabilities`
- `Buffer`
- `VertexArray`
- `Program`
- `Device`
- `ProgramBindings`
- `BufferBindingDesc`
- `FramebufferDesc`
- `ReadPixelsDesc`
- `BlitFramebufferDesc`
- `RenderPassDesc`
- `RenderStateDesc`
- `VertexArrayBindingDesc`
- `BufferReadDesc`
- `TextureReadDesc`

### 11.2 全局设备模型
`rhi/device.cc` 维护全局活动设备：

- `SetDevice()` 注入当前后端实现
- `GetDevice()` 提供统一访问入口
- `GetCapabilities()`、`GetSceneApiName()` 暴露设备元信息

这种全局单设备模型简单直接，适合当前单窗口、单渲染上下文的工程形态。

### 11.3 `Capabilities` 的双重角色
`Capabilities` 不只是设备自描述，还直接参与场景准入逻辑。`Playground` 用它决定：

- 哪些场景可以进入
- 哪些场景只能显示为 disabled
- tooltip 中要展示什么缺失能力

### 11.4 当前仍偏 OpenGL 的接口
虽然 RHI 已经统一了很多提交面，但 `Device` 中仍保留若干明显带 OpenGL 味道的接口，例如：

- `CreateProgram(const ProgramDesc&)`
- `Clear(const ClearDesc&)`

相比上一阶段，framebuffer 附件配置已经从这个遗留层中抽出，变成 `FramebufferDesc + ConfigureFramebuffer()`；这几轮又把 program 创建从 `CreateRenderProgram/CreateComputeProgram` 收口成了 `ProgramDesc + CreateProgram()`，把像素读回收敛成了 `ReadPixelsDesc`，并把 readback 目标从 color-only 扩展到显式 attachment type/index，把 framebuffer blit 收敛成了 `BlitFramebufferDesc`，把 viewport 从单独 setter 收回到 `RenderPassDesc/FramebufferState`，补齐了 `RenderPassDesc` 的 depth/stencil clear value，把 `SetClearColor() + Clear(mask)` 收口成单次 `ClearDesc + Clear()`，把 `VertexArray` 的常规配置路径收口成 `VertexArrayBindingDesc + ApplyBinding()` 并移除了逐项 attribute setter 的上层公有依赖面，把 buffer host readback 收口成 `BufferReadDesc + ReadData()`，把 texture host readback 收口成 `TextureReadDesc + ReadTextureData()`，并把长期存活 wrapper 的 buffer base 初始化收口成 `Device::ApplyBufferBindings()`；与此同时，`Program` 的 named uniform/texture 接口、独立激活入口与 program id 也已经一并收口到 `ApplyBindings()`。接下来的演进重点，是继续减少 renderer / playground 对“显式状态机式”设备调用的依赖。
这条线也开始延伸到 render pass 生命周期本身：RHI 现在提供一个轻量的 scoped render pass helper，把 `BeginRenderPass()/EndRenderPass()` 的配对关系收进对象生命周期里，`window` 与 `Framebuffer` 这类高层调用点不再必须手工拼接 begin/end。
同样的思路也开始延伸到 framebuffer state：RHI 现在提供 scoped framebuffer state helper，让 `CaptureFramebufferState()/RestoreFramebufferState()` 的配对关系也能收口进对象生命周期，`Framebuffer` 这类高层封装不再必须自己长期保存一份裸 state 成员。

与此同时，standalone draw 这条线也开始从“synthetic 但很松”收紧为更接近真实 graphics pipeline 的状态机：RHI 公有面已经把 `DrawArrays/DrawElements` 收口成单次 `DrawDesc + Draw()`；这条提交现在要求活动 `render pass`、活动 `Program`，并要求调用方在 `DrawDesc` 里显式提供本次 draw 所需的 `vertex array`，以及在 `DrawDesc.kind == kElements` 时显式提供 `index buffer`。renderer 侧则继续沿着同一方向，把 `Mesh::Submit()` 退化成 `BuildDrawDesc() + Draw()` 的便捷封装，并让 `RenderShader` / scene wrapper 优先通过 `ShaderProgram::DrawBindings()` 消费这份描述对象。

compute 这条线也开始从裸 `DispatchCompute + MemoryBarrier` 收口：RHI 公有面已经改成单次 `ComputeDispatchDesc` 提交，把 workgroup count 和 barrier 放进同一份描述对象；renderer 高层也已对齐成基于 `ComputeDispatchDesc` 的 `ShaderProgram::DispatchCompute()` 和 `ShaderProgram::DispatchComputeBindings()`，统一负责 program 激活、可选 bindings 提交与 compute dispatch，而 scene wrapper 侧也开始复用 `ComputeShader` 的一次性 dispatch helper，把原先分散的 `ApplyBindings() + SetWorkGroupNum() + Run()` 三步收回到单次提交。与此同时，standalone backend 也开始区分 render / compute program，并校验 dispatch 只能由活动 compute program 在非 render pass 环境下发起。

另一个被补实的点是 stencil：renderer 的 framebuffer attachment 定义不再把 stencil 写成 depth 占位，而是落到独立的 stencil attachment type 和 `Stencil8` 纹理格式；standalone backend 也开始真正 materialize stencil clear 与 stencil blit。

## 12. 后端实现设计

### 12.1 OpenGL backend
OpenGL 后端是当前最完整的实现，承担：

- shader 编译与绑定
- texture、framebuffer 和 draw
- `FramebufferDesc` 到 GL FBO attachment / draw buffer / complete check 的映射
- `ReadPixelsDesc` 到 GL read framebuffer / read buffer / tight pack readback 的映射
- `BlitFramebufferDesc` 到 GL read/draw framebuffer 与 attachment-selective blit 的映射
- depth / stencil attachment 到匹配 GL texture format 的映射
- 绝大多数场景的正确输出
- 作为其他后端路径的对照基线

### 12.2 Vulkan compat
`./run vk` 默认是：

```text
Vulkan presenter
  + OpenGL compatibility scene renderer
```

设计价值在于：

- 可以先把“窗口展示”迁移到 Vulkan
- 不阻塞现有场景继续运行
- 让 renderer/RHI 重构分阶段落地

定位上，`compat` 应被视为迁移期与回退期路径，而不是 Vulkan scene renderer 的终态。

### 12.3 Vulkan standalone / Synthetic RHI
`./run vk standalone` 的关键设计变化是：

- 不再创建 offscreen OpenGL context
- 场景侧改用独立 Vulkan backend
- 继续保留“presenter 接收一帧 RGBA 数据再展示”的总流程

当前它已经具备：

- 独立 Vulkan `Device`
- synthetic `Program`
- 以 `FramebufferDesc` 为输入的 synthetic framebuffer attachment record
- 以 `ReadPixelsDesc` 为输入的 synthetic framebuffer/default-buffer readback，包括常见 color 组合下按请求格式执行的 payload 转换
- synthetic default color/depth/stencil buffer
- 以 `BlitFramebufferDesc` 为输入的 attachment-selective synthetic blit，包括默认 framebuffer 与离屏附件之间的 depth/stencil 复制、带 origin 与 source/destination size 的复制/缩放，以及常见 color attachment 格式之间的 payload 转换
- resource、buffer binding、render pass、blit、readback 等基础语义
- 更接近真实 graphics pipeline 的 draw 前置状态校验：active render pass、active program、bound vertex array、`DrawDesc.kind == kElements` 时的 bound index buffer
- 更接近真实 compute pipeline 的 dispatch 前置状态校验：active compute program、非 render pass 环境、非零 workgroup
- depth/stencil clear value 到 synthetic depth/stencil attachment 的 materialize
- 对 synthetic blit 而言，目标 attachment 的格式契约不再通过覆写目标元信息来“伪装满足”，而是开始按目标格式执行真实 CPU 侧 payload 拷贝/转换

当前它仍缺少：

- 完整真实的 Vulkan graphics pipeline 体系
- 对所有现有场景的等价支持
- 与 OpenGL 路径一致的视觉保证

## 13. 测试设计

### 13.1 单元测试层
`test/` 目录负责更细粒度的逻辑验证，覆盖：

- `base`
- `geometry`
- `physics`
- `renderer`
- `rhi`

其中 `test/test_support/` 还提供 fake device 等测试辅助对象。

### 13.2 Playground 测试层
`playground/test/` 覆盖两类内容：

- 较接近实际资源/renderer 行为的测试
- `runtime_smoke.py` 驱动的运行时 smoke test

`runtime_smoke.py` 的设计很直接：

1. 启动 `window`
2. 写入运行时环境变量
3. 等待几秒让程序进入稳定状态
4. 收集日志输出
5. 校验关键 marker 是否存在

这种测试方式不验证画面像素正确性，但非常适合守住“程序还能启动、路径没有走错、关键日志仍然存在”这条底线。

### 13.3 串行约束
由于 smoke tests 共享图形运行时和构建产物，`playground/test/CMakeLists.txt` 显式将它们标记为串行执行。这是为了避免：

- 同时争用图形驱动
- 同时创建窗口造成不稳定
- 因日志或资源竞争导致假失败

## 14. 当前边界与继续演进建议

### 14.1 已经形成共识的方向
- 运行时选择应由脚本/环境变量决定，而不是塞进场景 UI
- 场景目录必须显式注册
- 资源路径和 shader 配置应统一进入 `Config`
- renderer 提交面要逐步收口到 bindings / render pass / render state
- Vulkan standalone 是 Vulkan scene renderer 的目标态，compat 是迁移/回退态
- Vulkan standalone 可以先“独立可启动并具备明确 draw / compute 状态约束”，再追求“完整可出图”

### 14.2 仍未完成的方向
- `rhi::Device` 仍保留部分 OpenGL 风格接口
- Vulkan standalone 仍是过渡态实现
- 并非所有历史 scene 源文件都已清理或纳入正式目录
- 仍有部分高层代码依赖 legacy 的 named-uniform 使用方式

### 14.3 建议的后续演进顺序
1. 继续把高层 renderer 的资源提交统一到同一套 binding 语义
2. 逐步减少 framebuffer attach 等偏 OpenGL 接口的暴露面
3. 在 Vulkan standalone 路径补齐更真实的 pipeline / descriptor / render pass 执行模型
4. 按场景逐个推进 Vulkan standalone 的稳定出图能力
