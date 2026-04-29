# playground/resource/texture/pbr_irradiance_tropical 需求

## 1. 目标
本目录保存 tropical 主题的 irradiance cubemap。

## 2. 当前使用方
- `TextureConfig` 名称：`pbr_irradiance_tropical`
- `PbrScene` 源码路径
- `PbrIrradianceCubemapGenerator` 的输出目标

## 3. 功能需求
- 应保留六面 irradiance 结果图。
- 应与环境 cubemap使用相同面命名约定。
