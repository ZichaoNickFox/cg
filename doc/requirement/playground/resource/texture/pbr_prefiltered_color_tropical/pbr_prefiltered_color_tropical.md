# playground/resource/texture/pbr_prefiltered_color_tropical 需求

## 1. 目标
本目录保存 tropical 主题的预过滤反射 cubemap。

## 2. 当前使用方
- `TextureConfig` 名称：`pbr_prefiltered_color_tropical`
- `PbrScene` 源码路径
- `PbrPrefilteredColorCubemapGenerator` 的输出目标

## 3. 功能需求
- 应同时表达多个 mip/roughness 级别和六个 cubemap 面。
- 文件名应显式带 `level` 前缀，便于程序按层加载。
