# playground/resource/model 需求

## 1. 目标
本目录保存 playground 运行时可加载的模型资源，是 `Config` 中 `ModelConfig` 的主要文件根目录。

## 2. 当前使用方
- 已注册场景直接使用：
  - `AngryBunnyScene` 使用 `bunny`
  - `ModelScene`、`SSAOScene` 使用 `cerberus`
  - `PathTracingScene`、`RTRTScene` 使用 `cornell_box_*`
- 当前未注册但源码中仍使用：
  - `PbrScene` 使用 `teapot`
  - `InstanceScene` 使用 `rock`、`planet`
- `nanosuit` 当前保留在配置中，但不是正式场景目录中的已注册模型输入

## 3. 功能需求
- 应保存 OBJ、FBX 及配套材质/纹理文件。
- 目录结构应允许 `Config` 与 `model_loader` 通过稳定相对路径定位模型。
- 根目录下允许同时存在“单文件模型”和“主题子目录模型”两种布局。
- 资源可服务已注册场景，也可作为未正式注册实验场景的备用资产，但文档应明确区分二者。

## 4. 变更约束
- 调整模型目录或文件名时，应同步更新 `playground/config.pb.txt` 中对应 `ModelConfig`。
- 对外部模型包的原始目录结构应优先保持兼容，减少 `mtl`/纹理相对路径失效风险。
