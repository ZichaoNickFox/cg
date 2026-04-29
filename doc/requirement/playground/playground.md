# playground 需求

## 1. 目标
`playground/` 目录负责把场景、窗口、输入、UI、运行时选择和测试入口组织成可执行的桌面实验程序。

## 2. 范围
- `window` 主程序
- `Playground` 运行时总控
- 场景注册与切换
- 资源配置入口
- scene/shader/resource/test 子目录组织

## 3. 功能需求
- 应提供 `window` 可执行程序。
- 应根据环境变量与脚本参数选择 OpenGL、Vulkan compat、Vulkan standalone 路径。
- 应维护显式场景目录并支持运行时切换、重载与能力门禁。
- 应加载 `config.pb.txt` 作为默认资源配置入口。
- `window` 应通过 RHI 统一读回接口获取最终 RGBA 帧，而不是直接拼接后端 read buffer 状态调用。
- 应提供与 playground 相关的测试与 smoke test。

## 4. 子目录职责
- `geometry_scene`：几何/数值实验场景
- `physics_scene`：物理实验场景
- `renderer_scene`：渲染实验场景与若干辅助场景源码
- `resource`：运行时模型、纹理与说明资源
- `shader`：playground 私有 GLSL
- `test`：靠近运行时的测试

## 5. 验收要点
- `./run` 能进入本目录产出的 `window` 程序。
- `Playground` 能完成场景切换与 UI 展示。
