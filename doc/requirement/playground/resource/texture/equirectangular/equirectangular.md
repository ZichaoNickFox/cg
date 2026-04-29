# playground/resource/texture/equirectangular 需求

## 1. 目标
本目录保存 equirectangular 环境图及其预览文件，供环境贴图转换或 PBR 预处理实验使用。

## 2. 当前使用方
- 当前作为 `equirectangular_tropical` 贴图配置的上游输入
- `Equirectangular2CubemapTool` 源码路径会把它转换为 `pbr_environment_tropical`

## 3. 功能需求
- 应保留 HDR 主资源与若干预览图。
- 文件名应保持同一环境主题的一致前缀，便于场景工具链引用。
