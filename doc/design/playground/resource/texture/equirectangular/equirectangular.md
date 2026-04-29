# playground/resource/texture/equirectangular 设计

## 1. 组成
当前以 `Tropical_Beach` 为主题，包含 `.hdr`、`.jpg`、`.ibl` 与预览图。

## 2. 设计要点
- 该目录通常作为 cubemap 生成工具和 PBR 环境贴图流程的上游输入。
- 与 `pbr_environment_tropical/` 的关系是“源环境图 -> 六面 cubemap 成果图”。
