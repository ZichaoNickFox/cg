# playground/resource/texture/pbr_irradiance_tropical 设计

## 1. 角色
本目录是环境贴图预处理链路中的 diffuse irradiance 输出。

## 2. 设计要点
- 与 `pbr_environment_tropical/` 保持同构目录结构，便于代码复用加载逻辑。
- 目录既可被场景直接读取，也可作为生成器输出产物被版本化保存。
