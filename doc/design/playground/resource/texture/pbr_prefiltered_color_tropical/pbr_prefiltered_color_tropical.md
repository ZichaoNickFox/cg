# playground/resource/texture/pbr_prefiltered_color_tropical 设计

## 1. 角色
本目录是 PBR 反射预过滤结果目录。

## 2. 设计要点
- 每个 `levelN_*` 组合对应一个 roughness/mip 层级的六面 cubemap 数据。
- 与 `pbr_environment_tropical/` 相比，这里额外编码了 mip/roughness 维度。
