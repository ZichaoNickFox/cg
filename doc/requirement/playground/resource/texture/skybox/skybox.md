# playground/resource/texture/skybox 需求

## 1. 目标
本目录保存传统 skybox 六面纹理。

## 2. 当前使用方
- `TextureConfig` 名称：`skybox`
- `playground/test/renderer_test.cc`
- `SkyboxScene` 这类源码路径可直接消费

## 3. 功能需求
- 应按 `px/py/pz/nx/ny/nz` 命名。
- 应可直接用于天空盒实验或基础 cubemap 验证。
