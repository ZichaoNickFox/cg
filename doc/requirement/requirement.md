# CG 软件需求规格说明书（SRS）

## 1. 引言

### 1.1 目的
本文档定义 `CG` 工程当前版本的软件需求，作为实现、测试、重构和文档同步的共同依据。本文档只描述仓库中已经存在或明确由当前代码支撑的能力，不把其他项目的模板内容或尚未完成的未来目标误写成已交付需求。

### 1.2 范围
`CG` 是一个本地桌面图形学实验工程，用于验证场景渲染、计算着色器实验、资源配置管理以及 `Renderer -> RHI -> Backend` 的演进方式。当前范围包括：

- 通过 `CMake` 与脚本构建并启动桌面程序
- 在运行时浏览、切换和重载实验场景
- 在 `OpenGL`、`Vulkan compat`、`Vulkan standalone` 路径下执行场景渲染
- 基于 `protobuf` 文本配置加载 shader、texture 与 model 资源
- 通过自动化测试验证核心抽象和关键运行路径

不在本文档范围内的能力见第 7 章。

### 1.3 预期读者
- 仓库维护者
- 图形学/渲染方向开发者
- 需要为新场景、新后端或新测试补充实现的贡献者

### 1.4 术语与缩写
- `Playground`：运行时总控模块，负责场景目录、帧循环、UI 和场景切换。
- `Scene`：一个可独立进入、更新、渲染和退出的实验场景。
- `Renderer`：场景使用的共享渲染资源、shader、repo 与辅助逻辑集合。
- `RHI`：渲染硬件接口抽象层，用于收敛高层对底层图形 API 的访问方式。
- `Presentation Runtime`：负责可见窗口展示的运行时，例如原生 OpenGL 或 Vulkan presenter。
- `Scene Renderer API`：场景实际执行渲染时所使用的 API，可与 presentation runtime 不同。
- `Vulkan compat`：Vulkan 负责展示，场景渲染仍由 OpenGL compatibility device 完成的模式。
- `Vulkan standalone`：不依赖 offscreen OpenGL context 的 Vulkan 场景路径。
- `Synthetic RHI`：当前 `Vulkan standalone` 使用的独立 RHI 实现，支持部分统一语义，但尚不是完整真实的 Vulkan raster renderer。

### 1.5 参考资料
- `README.md`
- `CMakeLists.txt`
- `CMakePresets.json`
- `run`
- `run.sh`
- `test.sh`
- `playground/window.cc`
- `playground/playground.cc`
- `renderer/scene.h`
- `renderer/scene.cc`
- `renderer/config.h`
- `renderer/config.cc`
- `renderer/proto/config.proto`
- `playground/config.pb.txt`
- `rhi/device.h`
- `playground/test/CMakeLists.txt`
- `playground/test/runtime_smoke.py`

### 1.6 文档组织
本 SRS 是总述文档。为避免根文档过度膨胀，`doc/requirement/` 目录按仓库一方目录结构递归镜像，重要子目录下分别维护局部文档。局部需求文档用于回答“该目录本身对外要稳定提供什么能力”。

除“覆盖范围一致”外，文档目录的组织方式也应与仓库根目录中的对应目录保持一致。也就是说，若根目录存在某个纳入文档范围的目录 `X/...`，则需求文档应在 `doc/requirement/X/.../` 下按相同层级组织；设计文档也应在 `doc/design/X/.../` 下按相同层级组织。

局部文档的文件名还应与其所在文件夹同名，而不是统一使用 `requirement.md` 或 `design.md`。也就是说：

- `doc/requirement/<dir>/<dir>.md`
- `doc/design/<dir>/<dir>.md`

仅两份总述文档保留：

- `doc/requirement/requirement.md`
- `doc/design/design.md`

例如：

- 根目录的 `playground/renderer_scene/`，对应 `doc/requirement/playground/renderer_scene/renderer_scene.md`
- 根目录的 `rhi/vulkan/shader/`，对应 `doc/design/rhi/vulkan/shader/shader.md`

当前镜像范围覆盖：

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

以及这些目录下的递归子目录。

以下目录不纳入递归需求文档范围：

- `thirdparty`：外部依赖，不属于本工程自研需求
- `build`：构建产物
- `log`：运行日志输出
- `.git`、`.vscode`、`doc`：仓库元数据或文档自身目录

## 2. 总体描述

### 2.1 产品视角
本系统是一个单机、本地运行的图形实验 playground，不依赖服务端或远程协作组件。它同时承担两类职责：

- 作为实验平台，快速验证阴影、SSAO、光线追踪、路径追踪、几何查询等场景
- 作为架构演进载体，持续把高层场景代码收敛到更统一的 renderer 与 RHI 抽象

### 2.2 产品功能概览
系统当前应提供以下能力：

- 构建并启动 `window` 可执行程序
- 根据命令行参数和环境变量选择运行模式
- 维护已注册场景目录，并在 UI 中进行选择与重载
- 根据设备能力禁用不可运行场景
- 读取配置文件并初始化 shader、texture、model 等资源
- 通过 `rhi::Device` 统一访问缓冲区、program、render pass、draw、compute 和 readback 能力
- 提供单元测试和运行时 smoke test

### 2.3 用户类别和特征
- 图形学开发者：关心 shader、材质、几何、路径追踪和后端差异
- 引擎/架构开发者：关心 renderer、RHI、backend 分层与可测试性
- 学习和实验用户：需要一个可以切换场景、观察效果并做小规模改动的本地工程

### 2.4 运行环境
- 本地桌面操作系统，当前开发体验以 macOS 为主
- `CMake >= 3.24`
- `C++20`
- `GLFW` 负责窗口和输入
- `ImGui` / `ImPlot` 负责调试 UI
- `OpenGL` 为必需后端
- `Vulkan` 为可选后端，仅在配置阶段发现 Vulkan SDK 且 `CG_ENABLE_VULKAN=ON` 时构建
- `Python 3` 用于运行时 smoke test
- `Mesa`/`llvmpipe` 为可选 offscreen OpenGL 运行时

### 2.5 设计与实现约束
- 当前最稳定、最完整的场景渲染基线仍是 `OpenGL`
- `Vulkan standalone` 当前是实验性独立场景路径，不要求与 OpenGL 达到视觉等价
- 场景可用性受设备能力约束，尤其是 `GLSL 450`、`compute shaders`、`storage buffers`
- 运行时和场景模式应在启动阶段确定，不要求在程序运行中热切换后端
- 资源配置由 `protobuf` 文本文件驱动，不要求当前版本提供图形化资源编辑器

### 2.6 用户文档
当前用户文档应至少包括：

- `README.md`：构建、运行和常见环境变量示例
- 本文档：需求基线
- `doc/design/design.md`：实现设计说明

### 2.7 假设与依赖
- 仓库子模块和第三方依赖已正确初始化
- 运行者具备本地编译 C++ 工程的基本能力
- `playground/config.pb.txt` 与其引用的资源文件在工作区内可访问
- 若请求 `CG_GL_RUNTIME=mesa`，则系统依赖构建时是否检测到兼容的 Mesa runtime；若没有，则需要降级

## 3. 外部接口需求

### 3.1 用户界面
系统应提供桌面窗口和基于 `ImGui` 的调试 UI，包括但不限于：

- `Scene Launcher`：显示当前场景、场景分类、可用性和切换入口
- 场景选择弹窗：显示分类列表、场景描述、能力状态和不支持原因
- `Inspector`：由具体场景或基类 `Scene` 展示运行参数与状态

### 3.2 输入接口
系统应接收并传递以下输入到场景逻辑：

- 键盘 `W` / `A` / `S` / `D`
- `ESC`
- 鼠标当前位置
- 鼠标左右键状态

当 GUI 捕获鼠标时，场景相机不得继续抢占鼠标控制。

### 3.3 软件接口
系统应暴露以下软件接口与约定：

- 构建接口：
  - `cmake --preset dev`
  - `cmake --build --preset dev`
  - `ctest --preset dev`
- 启动脚本：
  - `./run`
  - `./run.sh`
  - `./test.sh`
- 环境变量：
  - `CG_RUNTIME`
  - `CG_GL_RUNTIME`
  - `CG_VK_SCENE_MODE`
  - `CG_SCENE`
  - `CG_WINDOW_WIDTH`
  - `CG_WINDOW_HEIGHT`
- 日志接口：
  - 程序应将 glog 日志输出到 `log/log.txt`
- 配置接口：
  - 资源配置应通过 `renderer/proto/config.proto` 定义的消息结构，以及 `playground/config.pb.txt` 文本配置文件提供

### 3.4 硬件接口
- 系统应运行在支持所选图形后端的 GPU/驱动环境上
- OpenGL 路径应满足创建 core profile 上下文的要求
- Vulkan 路径应满足 `GLFW` 与 Vulkan surface/present 的基本要求
- 运行时 readback 路径应能从 framebuffer 或纹理读取 RGBA 像素数据用于 presenter

### 3.5 通信接口
当前版本无网络通信接口要求。

## 4. 系统特性与功能需求

### 4.1 构建与启动

#### FR-001 构建预设
系统应支持通过 `CMakePresets.json` 中的 `dev` 预设完成本地开发构建。

#### FR-002 启动脚本
系统应提供 `./run` 脚本，并在启动前自动执行：

- `cmake --preset dev`
- `cmake --build --preset dev --target window`

#### FR-003 启动别名脚本
系统应提供 `./run.sh` 作为 `./run` 的转发入口。

#### FR-004 运行命令格式
系统应支持以下启动形式：

- `./run`
- `./run gl`
- `./run vk`
- `./run vk compat`
- `./run vk standalone`
- `./run --help`

其中：

- `gl` / `opengl` 选择 OpenGL 运行时
- `vk` / `vulkan` 选择 Vulkan 运行时
- `compat` / `gl` 选择 Vulkan compatibility scene mode
- `standalone` / `vk-scene` 选择 Vulkan standalone scene mode

#### FR-005 默认运行时选择
当未显式设置 `CG_RUNTIME` 且未在命令行中给出运行时参数时：

- 若构建产物包含 Vulkan RHI，则系统应默认选择 Vulkan runtime
- 否则系统应默认选择 OpenGL runtime

#### FR-006 窗口尺寸配置
系统应支持通过 `CG_WINDOW_WIDTH` 与 `CG_WINDOW_HEIGHT` 设置窗口尺寸；无配置时应使用默认尺寸。

#### FR-007 OpenGL runtime 选择
系统应支持以下 OpenGL runtime 选择规则：

- `CG_GL_RUNTIME=native` 时，强制使用 native OpenGL 路径
- `CG_GL_RUNTIME=mesa` 时，请求使用 Mesa offscreen 路径；若不可用，则允许降级
- `CG_GL_RUNTIME` 未设置时，若当前构建集成了 Mesa offscreen runtime，则默认选择 Mesa；否则默认选择 native OpenGL

#### FR-008 日志目录
系统启动时应确保存在 `log/` 目录，并将运行日志写入其中。

### 4.2 场景目录与场景生命周期

#### FR-101 已注册场景目录
系统应维护一个显式注册的场景目录，而不是自动扫描文件系统。当前版本注册的场景至少包括：

- `FittingScene`
- `ImPlotDemoScene`
- `ModelScene`
- `ShadowScene`
- `SampleScene`
- `GeometryScene`
- `RayTracingScene`
- `PathTracingGeometryScene`
- `PathTracingScene`
- `RTRTScene`
- `SSAOScene`
- `AngryBunnyScene`

#### FR-102 场景分类
场景目录应支持分类展示。当前版本应至少包含：

- `Rendering`
- `Physics`
- `Geometry`

`Animation` 分类可被保留为扩展槽位，但当前版本不要求其中已有场景。

#### FR-103 默认场景
未指定 `CG_SCENE` 时，系统应使用 `FittingScene` 作为默认初始场景。

#### FR-104 初始场景选择
系统应支持通过 `CG_SCENE` 指定初始场景 ID。

#### FR-105 场景切换
系统应支持在运行过程中通过 UI 切换场景，并支持重载当前场景。

#### FR-106 场景可用性校验
系统在进入场景前应依据 `rhi::Capabilities` 对场景进行准入校验，至少校验以下能力：

- `supports_glsl_450`
- `supports_compute`
- `supports_storage_buffers`

#### FR-107 不可用场景处理
若指定场景不存在或当前设备不满足其能力要求，系统应记录日志并在初次进入失败时回退到默认场景。

#### FR-108 场景生命周期
每个场景应至少支持以下生命周期：

- `OnEnter`
- `OnUpdate`
- `OnRender`
- `OnExit`

基类 `Scene` 应提供 `Enter`、`Update`、`Render`、`Exit` 包装调用。

### 4.3 交互与调试能力

#### FR-201 键鼠输入传递
系统应在每帧将基础键鼠输入写入 `Io`，供当前场景读取。

#### FR-202 默认相机控制
基类 `Scene` 应提供默认相机移动与旋转逻辑：

- `W/S` 前后移动
- `A/D` 左右移动
- 鼠标左键拖动旋转视角
- `ESC` 触发退出

#### FR-203 GUI 鼠标抢占
当 ImGui 报告 `WantCaptureMouse` 为真时，场景相机控制应停止处理鼠标旋转。

#### FR-204 场景选择器信息
场景选择器应显示当前 renderer/runtime 信息、shader language 和关键 capability 状态，并支持显示不支持场景及其原因。

### 4.4 资源配置与内容加载

#### FR-301 配置模型
系统应通过 `renderer/proto/config.proto` 定义以下资源配置模型：

- `ShaderConfig`
- `TextureConfig`
- `ModelConfig`
- `ConfigData`

#### FR-302 默认配置文件
系统应使用 `playground/config.pb.txt` 作为默认资源配置文件。

#### FR-303 配置解析
系统应支持从 protobuf 文本格式读取配置数据，并将 shader、texture、model 配置加载到内存映射结构中。

#### FR-304 路径解析策略
系统应支持以下资源路径解析顺序：

1. 原始路径已是绝对路径，或在当前工作区直接存在
2. 相对项目根目录存在
3. 相对配置文件目录存在

#### FR-305 Scene 资源初始化
`Scene::Enter` 应基于配置初始化至少以下共享 repo：

- `ShaderProgramRepo`
- `TextureRepo`
- `ObjectRepo`
- `MeshRepo`
- `MaterialRepo`
- `PrimitiveRepo`
- `LightRepo`
- `ShadowRepo`
- `BVH`

### 4.5 渲染抽象与 RHI

#### FR-401 统一设备入口
高层 renderer 与 scene 代码应优先通过 `rhi::Device` 访问图形能力，而不是直接依赖具体后端实现。

#### FR-402 能力查询
RHI 应暴露 `Capabilities`，至少包含：

- `backend`
- `scene_backend`
- `graphics_api_name`
- `scene_api_name`
- `shader_language_name`
- `supports_glsl_450`
- `supports_storage_buffers`
- `supports_storage_images`
- `supports_compute`

#### FR-403 核心 RHI 能力
RHI 应支持以下核心抽象或操作：

- `Buffer`
- `VertexArray`
- `Program`
- render program 与 compute program 创建
- texture 上传与数据读回
- storage texture 绑定
- buffer bindings 提交
- buffer host readback descriptor
- texture host readback descriptor
- compute dispatch descriptor
- framebuffer 创建、绑定、描述式配置与 blit
- render pass begin/end
- render state 提交
- `DrawDesc` / `Draw()`
- `ReadPixels`

#### FR-404 ProgramBindings
系统应支持通过一次 `Program::ApplyBindings()` 批量向 program 提交：

- uniforms
- sampled textures
- storage textures
- buffers

当一次提交本身已经定义了完整的 program 资源状态时，高层 renderer 也应直接通过统一的 `ApplyBindings()` 提交入口完成一次性提交，而不是把“显式激活 + 资源下发”拆散到业务层手动拼接。`renderer::ShaderProgram::ApplyBindings()` 本身应承担 program 激活与 bindings 下发，避免调用方隐式依赖“上一处刚好已经激活过”。
其中 storage texture 绑定也应优先经由 `ProgramBindings` / `Program::ApplyBindings()` 兑现，而不是要求上层依赖单独的 `Device::BindStorageTexture()` 公有接口。
`rhi::Program` 的公有提交面应收敛到单一的 `ApplyBindings()`，而不是继续对上层暴露单独的 program 激活入口、program id 或 named uniform/texture setter。
与这条提交收口方向一致，renderer 侧的 program 创建也应采用单一的 `ShaderProgramDesc`，并内部映射到 `rhi::ProgramDesc + CreateProgram()`，而不是继续为 render / compute 保留两套高层构造模式。
`renderer::RenderShader` 与 `renderer::ComputeShader` 这类高层包装也不应继续保留 `SetModel()`、`SetCamera()`、`SetMaterial()`、`SetWorkGroupNum()`、`Run()` 这类状态机式提交通道；render path 应通过 batched `ShaderProgramBindings + DrawBindings()` 收口，compute path 则应通过基于 `ComputeDispatchDesc` 的一次性 dispatch helper 收口。
当 `RenderShader` / `ComputeShader` 需要附带 scene-common repo/buffer/texture bindings 时，这部分 common bindings 也应被缓存并合并进最终单次 draw/dispatch 提交，而不是在 wrapper 构造阶段先独立预提交一次。
对于不直接触发 draw、而只是补齐 render program 资源状态的 wrapper，也应继续通过 `RenderShader` 自身合并 common bindings 后的 `ApplyBindings()` 提交，而不是重新绕回裸 `ShaderProgram` 或 backend program 句柄。

对于 camera、model、material index、primitive start index、resolution、frame num、dirty 这类高频但结构稳定的提交项，renderer 还应提供可复用的 bindings 追加/helper 入口，使 scene 和 shader 包装层能够把这些状态合并进同一次 `ShaderProgramBindings` 提交，而不是重复散落出一批小的 setter 调用。对于 render pass 中最常见的 `model + camera` 初始状态，还应进一步提供组合型 render-object helper，避免各个 shader/scene 再次手写同一组拼接顺序。
对于仍使用 legacy `model/view/project` 与 `view_pos_ws` uniform 约定的 render shader，renderer 也应提供对应 helper，并要求 wrapper 继续通过 batched bindings 一次性兑现这组旧约定。
当一次 render 提交已经同时掌握完整 bindings 与 draw 参数时，renderer 还应优先通过 `ShaderProgram::DrawBindings()` 把 bindings 与 `DrawDesc` 一起下发，而不是重新回退成 `ApplyBindings()` 后再手工 `Submit()` 的两步提交流程。
与这条收口方向一致，RHI 自身也应提供把 `ProgramBindings + DrawDesc`、`ProgramBindings + ComputeDispatchDesc` 一起提交的 convenience 入口，避免这类“一次激活并提交”的语义只散落在 renderer 自己维护。

#### FR-404A VertexArray 绑定描述
`VertexArray` 的常规配置路径也应收口到 `VertexArrayBindingDesc + ApplyBinding()`，用于一次性表达 vertex buffer 来源与 attribute 列表，而不是要求 renderer / mesh 业务层继续手工拼 `Bind()`、`EnableAttribute()`、`SetFloatAttribute()`、`SetAttributeDivisor()`。
与这条收口方向一致，逐项 attribute setter 不应继续作为上层常规依赖的 `VertexArray` 公有接口存在。

#### FR-405 Buffer 绑定收口
高层 renderer 应优先通过 `ProgramBindings` / `ApplyBindings()` 这类统一入口提交缓冲区绑定；对于 `SSBO`、`AutomicCounter` 这类长期存活的 wrapper，也应优先通过 `Device::ApplyBufferBindings()` 这类显式 descriptor 入口完成一次性初始化，而不是在业务层散落直接 `BindBase()`。
`BindBase()` 仍可作为 backend / 低层对象内部原语存在，但不应再被上层业务 wrapper 当成常规初始化入口；测试统计里的显式 buffer-binding 提交也应只代表 `ApplyBufferBindings()` 路径，而不是把任意 direct `BindBase()` 混进同一语义。
buffer 的 host 侧读回应优先通过 `BufferReadDesc + ReadData()` 这类显式接口完成，而不是继续要求 renderer / 测试直接依赖裸 `Map()/Unmap()`。
texture 的 host 侧读回应优先通过 `TextureReadDesc + ReadTextureData()` 这类显式接口完成，而不是继续让上层重复传裸 `level/size` 参数组合。
`TextureRepo::AsTexture2DArray()` 这类会被 `ProgramBindings` 持有 texture 地址的接口，应返回稳定的 repo-owned/cached `Texture` 引用，而不是临时值，避免 bindings 内部保存悬垂 texture 指针。
`Scene::GetTexture()` 与 `Framebuffer::GetTexture()` 这类同样会被 scene/shader wrapper 继续传入 `ShaderProgramBindings` 的查找接口，也应作为纯查找入口返回稳定的 owned `Texture` 引用，而不是返回按值拷贝的临时 `Texture` 或引入额外的 reload 模式分支。
公共 render state 也应通过 `RenderStateDesc + ApplyRenderState()` 统一提交，而不是继续把 depth test、cull enable、cull mode、front face 拆成上层常规接口。
当高层只需要在局部作用域里临时覆盖 render state 时，也应通过 `CaptureRenderState()` / `RestoreRenderState()` 或 `ScopedRenderState` 收口，而不是继续在业务代码里手工写成“先关闭，再重新打开”的成对状态切换。
viewport 也应作为 `RenderPassDesc` 或 `FramebufferState` 的组成部分提交/恢复，而不是继续依赖单独的公有 viewport setter。

#### FR-406 离屏 framebuffer 描述
系统应支持使用 `FramebufferDesc` 一次性描述用户创建 framebuffer 的附件集合，并由后端在配置阶段完成：

- attachment 绑定
- color draw buffer 选择
- framebuffer 完整性检查

`FramebufferDesc` 当前至少应支持 2D color/depth/stencil attachment。
其中 stencil attachment 应使用独立的 attachment type 与匹配的 stencil texture format，而不是复用 depth attachment 占位。
当后端支持 split read/draw framebuffer 绑定时，写侧行为（如 clear、color attachment clear、后续 render pass 目标）应跟随当前 draw framebuffer，而不是误用旧的全局 framebuffer 绑定；这类语义应通过 `CaptureFramebufferState()` / `RestoreFramebufferState()` 和 render pass 路径对外暴露，而不是要求上层依赖单独的 `BindFramebuffer(...)` 公有接口。
对于非 attachment-selective 的 clear，系统还应支持通过一次 `ClearDesc` 显式提交 clear mask、color、depth clear value 与 stencil clear value，而不是依赖 `SetClearColor()` 一类拆散的隐式设备状态。
当 render pass 在非默认 framebuffer 上请求 attachment-selective 的 color clear 时，目标也应只允许引用真实存在的 color attachment；默认 framebuffer 路径只允许 attachment 0，而非默认 framebuffer 不应静默接受未挂载的 color attachment index。
当后端支持 `CaptureFramebufferState()` / `RestoreFramebufferState()` 时，state 至少应保留 viewport，以及当前 read framebuffer 与 draw framebuffer 绑定关系；兼容字段若存在，应以 draw framebuffer 为别名。
系统还应支持 scoped framebuffer state helper，把 framebuffer state 的 capture/restore 生命周期收口到单个对象作用域，而不是要求高层所有调用点都手工保存和恢复 state 对象。
在 renderer 层，`Framebuffer` 也应继续把 `Bind()/Unbind()` 收口成 scoped helper，让 scene/pass 代码优先依赖对象作用域而不是手工维护 bind/unbind 配对；`BindScoped()` 应成为业务层正式 contract，而 `Bind()/Unbind()` 退回 renderer 内部生命周期细节。
当 scene/pass 需要在作用域结束前提前结束离屏 pass 时，`Framebuffer::ScopedBind` 也应支持显式 reset，并立即完成 render pass 结束与 framebuffer state 恢复，而不是要求业务层重新引入裸 `Unbind()`。

#### FR-407 主 render pass 抽象
系统应支持使用 `RenderPassDesc` 描述默认 framebuffer 的 viewport、color clear、depth clear 和 stencil clear 语义，并允许显式指定 depth/stencil clear value。
系统还应支持使用 scoped render pass helper 把 `BeginRenderPass()/EndRenderPass()` 生命周期收口到单个对象作用域，而不是强制高层所有调用点都手工维护 begin/end 配对。
`RenderPassDesc.viewport_size` 应表示正尺寸 viewport；后端不应接受零或负的 viewport 宽高进入 render pass 生命周期。
当 `RenderPassDesc.framebuffer` 指向非默认 framebuffer 时，后端应要求该 framebuffer 已存在且处于 complete 状态，而不是在不完整 attachment 集合上继续开始 render pass。
当 `RenderPassDesc.color_attachments` 声明 color target 时，默认 framebuffer 路径只允许 attachment 0；非默认 framebuffer 路径则应要求每个声明的 color attachment index 都真实挂载在目标 framebuffer 上，而不是静默忽略不存在的 attachment。
当非默认 framebuffer render pass 启用 `clear_depth` 或 `clear_stencil` 时，后端也应要求 depth/stencil attachment 真实存在，而不是静默跳过对应 clear。

对于 `DrawDesc + Draw()`，系统应要求调用方把本次 draw 所需的 `vertex array` 与 `index buffer` 显式挂进描述对象，而不是继续依赖业务层预先 `Bind()` 的隐式几何状态。
renderer 侧的 mesh 也应支持先物化可复用的 `DrawDesc`，再由上层决定是直接 `Draw()`，还是通过 `ShaderProgram::DrawBindings()` 与 program bindings 一起提交；`Mesh::Submit()` 应只是对这条描述式路径的便捷封装，而不是唯一入口。

#### FR-408 像素读回抽象
系统应支持使用 `ReadPixelsDesc` 一次性描述像素读回请求，并由后端在一次调用内完成：

- framebuffer 选择
- 默认 framebuffer 的 front/back 选择
- framebuffer attachment type / attachment index 选择
- 紧凑的 RGBA/深度等像素读回

其中默认 framebuffer 的 front/back 选择仅对 color readback 生效；depth/stencil 读回不依赖该选择。
当后端以 standalone synthetic 模式 materialize 默认 framebuffer 时，front color 应代表上一份已完成的默认 framebuffer 结果，back color 应代表当前默认 framebuffer 的活动写目标。
当 color readback 的请求格式与底层 attachment/default color buffer 的实际存储格式不同，但仍属于工程允许的常见 synthetic color 组合时，后端应优先保持请求侧的读回格式契约，并完成必要的 payload 转换。
当 `ReadPixelsDesc` 指向默认 framebuffer 时，后端只应接受 attachment 0；当它指向非默认 framebuffer 时，后端应要求目标 attachment 真实挂载，而不是静默返回全零或空结果。

#### FR-409 framebuffer blit 抽象
系统应支持使用 `BlitFramebufferDesc` 一次性描述 framebuffer blit 请求，并由后端在一次调用内完成：

- source / destination framebuffer 选择
- 当 source 为默认 framebuffer color 时的 front/back 选择
- 当 destination 为默认 framebuffer color 时的 front/back 选择
- source / destination color attachment 选择
- source / destination origin 选择
- source / destination size 选择
- blit mask 与 filter 选择

当 mask 包含 `depth` 或 `stencil` 时，后端也应尽可能保持对应 attachment 的复制语义。
当 source 或 destination 为默认 framebuffer 时，这一语义同样应尽可能保持。
其中默认 framebuffer 的 front/back 选择仅对 color blit 生效；default depth/stencil 的 source / destination 都不依赖该选择。
当 `BlitFramebufferDesc` 引用 color/depth/stencil attachment 时，后端应分别校验 source 与 destination 的 attachment 合法性；默认 framebuffer 路径只允许 attachment 0，非默认 framebuffer 路径则不应静默接受未挂载 attachment。
当 `read_origin` 或 `draw_origin` 为负时，后端应将其解释为部分越界的 rect，并继续按同一映射关系裁剪有效区域，而不是直接拒绝或退化为空操作；这一语义同样适用于 default framebuffer 尚未 materialize、需要按 blit 请求惰性确定尺寸的路径。
当 color blit 的 source / destination size 不同时，后端应将其解释为缩放 blit，并继续通过 `filter` 表达采样语义。
当 source rect 或 destination rect 部分超出各自 framebuffer 边界时，后端应按同一映射关系同时裁剪 source / destination 的有效区域，而不是把完整 source 强行缩放进被截断后的 destination。
当 color blit 的 source / destination 指向同一份底层存储且复制区域重叠时，后端不应因为原地覆写而污染尚未读取的 source 区域。
当 default depth/stencil blit 的 source / destination 指向同一份默认 framebuffer 存储且复制区域重叠时，后端也不应因为原地覆写而污染尚未读取的 source 区域。
当 color attachment 的 source / destination 格式不同但仍属于工程允许的常见组合时，后端应优先保持 destination 格式契约，而不是通过篡改 destination attachment 的缓存元信息来伪造结果。

#### FR-410 公共 GPU 数据同步
当设备支持 `storage buffers` 时，`Scene::Update` 应同步以下共享数据到 GPU：

- `MaterialRepo`
- `BVH`
- `PrimitiveRepo`
- `LightRepo`

若设备不支持 `storage buffers`，系统应记录明确告警。

#### FR-411 图形 draw 前置状态
系统应要求图形 draw 提交满足最基本的 graphics pipeline 前置状态约束：

- `DrawDesc` / `Draw()` 应发生在活动 `render pass` 内
- draw 前应已有活动 `Program`
- `DrawDesc.vertex_array` 应在提交时显式可用
- 当 `DrawDesc.kind == kElements` 时，`DrawDesc.index_buffer` 也应在提交时显式可用

#### FR-412 compute dispatch 前置状态
系统应要求 compute dispatch 满足最基本的前置状态约束：

- 提交 `ComputeDispatchDesc` 前应已有活动 compute `Program`
- `ComputeDispatchDesc` 不应发生在活动 `render pass` 内
- workgroup 维度应为非零正数
- compute barrier 应作为同一次 `ComputeDispatchDesc` 提交的一部分表达，而不是继续暴露成单独的 `MemoryBarrier()` 公有设备调用
- 高层 renderer 应优先通过基于 `ComputeDispatchDesc` 的 `ShaderProgram::DispatchCompute()`、`ShaderProgram::DispatchComputeBindings()`，以及 `ComputeShader` 侧等价的一次性 dispatch helper 触发 dispatch 与 barrier，而不是散落直接后端调用

### 4.6 运行模式与后端行为

#### FR-501 OpenGL native 路径
当运行时选择 OpenGL 且未启用 Mesa offscreen runtime 时，系统应：

- 创建可见 OpenGL 窗口
- 在该窗口中直接执行渲染与 UI
- 使用 `glfwSwapBuffers` 展示结果

#### FR-502 OpenGL Mesa 路径
当运行时选择 OpenGL 且启用 Mesa offscreen runtime 时，系统应：

- 在 offscreen OpenGL context 中完成场景渲染
- 将 RGBA 像素数据交给 on-screen presenter 展示

#### FR-503 Mesa 降级
当显式请求 `CG_GL_RUNTIME=mesa` 但当前构建未集成可用 Mesa runtime 时，系统应记录降级日志并回退到 native OpenGL 路径。

#### FR-504 Vulkan compat 路径
当运行时选择 Vulkan 且 scene mode 为 compat 时，系统应：

- 使用 Vulkan presenter 创建并展示窗口
- 使用 OpenGL compatibility device 完成场景渲染
- 将 offscreen RGBA 渲染结果交给 Vulkan presenter 展示

#### FR-505 Vulkan standalone 路径
当运行时选择 Vulkan 且 scene mode 为 standalone 时，系统应：

- 不依赖 offscreen OpenGL context
- 初始化独立 Vulkan scene backend
- 作为 Vulkan scene renderer 的目标态
- 支持当前 RHI 所要求的基础资源、render pass、buffer bindings、program bindings、draw 前置状态与 readback 语义

#### FR-506 Vulkan standalone 当前边界
当前版本不要求 `Vulkan standalone` 为完整真实的 Vulkan raster renderer，也不要求所有场景达到与 OpenGL 等价的视觉输出。

#### FR-507 Vulkan compat 路径定位
`Vulkan compat` 应被视为迁移期与回退期路径，而不是长期目标态。`standalone` 成熟后，`compat` 主要保留兼容与排障价值。

#### FR-508 运行时识别日志
系统启动时应输出足够区分运行路径的日志，至少包括：

- `Presentation Runtime`
- `Vulkan Scene Mode`（Vulkan 路径下）
- `Offscreen GL Runtime`
- `RHI Runtime API`
- `Scene Renderer API`
- `Scene Capabilities`

### 4.7 测试与可验证性

#### FR-601 单元测试
系统应提供单元测试覆盖至少以下领域：

- `base`
- `geometry`
- `physics`
- `renderer`
- `rhi`

#### FR-602 Playground 测试
系统应提供 `playground/test` 目录下的测试，用于验证 renderer 行为和运行时启动路径。

#### FR-603 runtime smoke test
系统应提供运行时 smoke test，至少覆盖：

- `runtime_smoke_opengl_native`
- `runtime_smoke_vulkan_native`（仅当 Vulkan RHI 被构建）
- `runtime_smoke_vulkan_standalone`（仅当 Vulkan RHI 被构建）

若 Mesa 可用，还应覆盖 Mesa 路径；若 Mesa 不可用，还应覆盖 fallback 行为。

#### FR-604 串行执行
共享窗口、图形驱动与构建产物的 smoke tests 应以串行方式运行，避免相互干扰。

#### FR-605 统一测试脚本
系统应提供 `./test.sh`，并通过 `cmake --preset dev`、`cmake --build --preset dev`、`ctest --preset dev` 完成测试流程。

#### FR-606 standalone draw 状态回归
当 Vulkan 后端被构建时，`test/rhi/vulkan_device_test.cc` 应覆盖 standalone draw 的关键前置状态边界，至少包括：

- render pass 未激活时应拒绝 draw
- 未通过 `ApplyBindings()` 激活 program 时应拒绝 draw
- `DrawDesc.vertex_array` 缺失时应拒绝 draw
- `DrawDesc.kind == kElements` 且 `DrawDesc.index_buffer` 缺失时应拒绝提交

#### FR-607 standalone compute 状态回归
当 Vulkan 后端被构建时，`test/rhi/vulkan_device_test.cc` 应覆盖 standalone compute dispatch 的关键前置状态边界，至少包括：

- 未通过 `ApplyBindings()` 激活 compute program 时应拒绝 dispatch
- render program 处于活动状态时应拒绝 dispatch
- 活动 render pass 内应拒绝 dispatch
- 零 workgroup 维度应拒绝提交

## 5. 非功能需求

### 5.1 可维护性

#### NFR-001
需求、设计与实现变更后应保持同步，避免继续引用其他工程的模板内容或无效路径。

#### NFR-002
新增渲染能力应优先接入现有 renderer/RHI 抽象，而不是直接把后端实现暴露到高层场景逻辑。

#### NFR-003
场景注册应保持显式、可追踪，避免“文件存在但不属于当前交付能力”的隐式行为混入需求基线。

### 5.2 可测试性

#### NFR-101
关键运行模式应可通过命令行和自动化测试稳定复现。

#### NFR-102
RHI 抽象应允许通过 fake device、logic test 或 synthetic backend 验证状态和提交流程。

### 5.3 可移植性

#### NFR-201
在未检测到 Vulkan SDK 时，工程仍应能以纯 OpenGL 路径完成构建。

#### NFR-202
系统应允许在有无 Mesa 的构建环境下运行，并在能力不足时优雅降级。

#### NFR-203
后端抽象应为未来继续补齐真实 Vulkan scene renderer 预留演进空间。

### 5.4 可诊断性

#### NFR-301
运行时日志应能区分 presentation backend、scene backend、scene mode、shader language 与 capability 状态。

#### NFR-302
场景不可用、Mesa 回退和设备切换等关键路径应提供明确日志。

## 6. 验收准则

### 6.1 构建验收
- 运行 `cmake --preset dev` 与 `cmake --build --preset dev` 后，应生成 `window` 可执行程序。
- 运行 `./run` 时，应能够自动完成配置、构建和启动。

### 6.2 启动验收
- 运行 `./run gl` 时，应成功进入 OpenGL 路径并输出 OpenGL 相关日志。
- 当 Vulkan RHI 被构建时，运行 `./run vk` 时应输出 `Presentation Runtime : Vulkan`。
- 当 Vulkan RHI 被构建时，运行 `./run vk standalone` 时应输出 `Vulkan Scene Mode : Standalone Vulkan RHI`，并且不再声明使用 offscreen OpenGL runtime。

### 6.3 场景验收
- 未设置 `CG_SCENE` 时，应进入 `FittingScene`。
- 设置有效 `CG_SCENE` 时，应进入对应场景，或在能力不足时给出明确原因。
- 设置无效 `CG_SCENE` 时，应记录日志并回退到默认场景。

### 6.4 测试验收
- 运行 `ctest --preset dev` 时，已启用的单元测试应通过。
- `playground/test` 中已启用的 smoke tests 应通过。

## 7. 非范围
- 生产级编辑器、资源管理后台和发布打包流程
- 网络协作、远程渲染或服务端渲染能力
- 当前版本内完成所有场景的纯 Vulkan 真实渲染等价实现
- 在运行中热切换图形后端
