# playground/resource/model/cerberus/Textures 设计

## 1. 组成
当前包含 `Cerberus_A/M/N/R.tga`，对应一组典型 PBR 贴图通道。

## 2. 设计要点
- 目录是 Cerberus 主题资源包的一部分，不单独在配置中暴露。
- 贴图文件名遵循资源包原始约定，而不是本工程二次命名。

## 3. 边界
- 本目录只存贴图，不存 shader 或材质逻辑。
