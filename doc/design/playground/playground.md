# playground 设计

## 1. 角色
`playground/` 是应用层装配目录，负责把下层 renderer/RHI 能力组合成可运行的桌面程序。

## 2. 主要文件
- `window.cc`：程序入口、运行时分流、presenter 与主循环
- `playground.h/.cc`：场景目录、场景切换、UI、帧生命周期
- `config.pb.txt`：默认资源配置
- `shaders.cc`：部分 shader 相关辅助

## 3. 设计要点
- `cg_playground` 作为静态库承载运行时总控，`window` 作为最终可执行目标。
- 本目录通过子目录进一步把场景、资源、shader 与测试分层。
- 运行时选择不放在 UI 中，而是在程序启动前由脚本和环境变量完成。
- `window.cc` 在 present/readback 路径上会显式把 `ReadPixelsDesc.attachment_type` 设为 `kColor`，并表达默认 framebuffer 的 color/front-back 读回需求，避免把 OpenGL `glReadBuffer` 细节散落在应用层；更细的 attachment type/index 语义留在 RHI 层统一承接。

## 4. 边界
- 场景内部渲染细节留在 `renderer/` 与各 scene 子目录中。
- 图形 API 抽象不在本目录实现，而是依赖 `rhi/`。
