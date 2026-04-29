# playground/shader/shadow_scene 需求

## 1. 目标
本目录保存 `ShadowScene` 相关 shader。

## 2. 当前使用方
- 已注册场景：`ShadowScene`
- 对应配置名：
  - `shadow_scene`
  - `shadow_scene_depth`

## 3. 功能需求
- 应包含主渲染、深度图生成以及 PCF/PCSS 共用逻辑。
- 共用逻辑应以可 include 的 `.glsl` 文件形式组织。
