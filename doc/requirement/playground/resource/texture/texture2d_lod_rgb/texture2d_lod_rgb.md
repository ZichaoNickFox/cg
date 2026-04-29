# playground/resource/texture/texture2d_lod_rgb 需求

## 1. 目标
本目录提供用于 2D 纹理 LOD 验证的多级 RGB 数据。

## 2. 当前使用方
- `TextureConfig` 名称：`texture2d_lod_rgb`
- `TextureLodScene` 源码路径（当前未正式注册）

## 3. 功能需求
- 应按 `level0/level1/level2` 这类名字表达 mip 级别。
- 应可被 texture LOD 相关实验直接加载。
