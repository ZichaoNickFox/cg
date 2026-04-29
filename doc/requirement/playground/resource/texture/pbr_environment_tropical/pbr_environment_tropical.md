# playground/resource/texture/pbr_environment_tropical 需求

## 1. 目标
本目录保存 tropical 主题的环境 cubemap。

## 2. 当前使用方
- `TextureConfig` 名称：`pbr_environment_tropical`
- `SkyboxScene`、`PbrScene` 源码路径
- `PbrIrradianceCubemapGenerator`、`PbrPrefilteredColorCubemapGenerator` 的输入

## 3. 功能需求
- 应按 `px/py/pz/nx/ny/nz` 命名六面。
- 应可直接作为 skybox 或环境反射输入。
