# renderer/shader/pbr 需求

## 1. 目标
本目录保存 PBR 着色、环境贴图预处理和 BRDF 相关 shader。

## 2. 当前使用方
- 顶层配置名：
  - `pbr`
  - `pbr_irradiance_cubemap_generator`
  - `pbr_prefiltered_color_cubemap_generator`
  - `pbr_BRDF_integration_map_generator`
- 当前主要消费者是未正式注册但仍保留的 `PbrScene` 与若干 cubemap 生成工具场景/源码路径

## 3. 功能需求
- 应覆盖主 PBR 渲染与 IBL 预处理的核心 shader。
- 应把 `NDF`、`fresnel`、`geometry`、`material` 等公共逻辑拆成可复用 include。
- 顶层 shader 与公共 include 应共存，但职责要清晰分离。
