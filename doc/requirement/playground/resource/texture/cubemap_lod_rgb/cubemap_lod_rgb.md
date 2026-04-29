# playground/resource/texture/cubemap_lod_rgb 需求

## 1. 目标
本目录提供用于 cubemap LOD 验证的多面多级 RGB 纹理数据。

## 2. 当前使用方
- `TextureConfig` 名称：`cubemap_lod_rgb`
- `playground/test/renderer_test.cc`
- `TextureLodScene` 源码路径（当前未正式注册）

## 3. 功能需求
- 应按 `levelX_face` 方式命名，显式区分 mip 级别和 cubemap 面。
- 应至少覆盖多个 mip level，便于观察采样层级差异。
