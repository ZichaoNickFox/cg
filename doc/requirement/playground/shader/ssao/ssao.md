# playground/shader/ssao 需求

## 1. 目标
本目录保存 `SSAOScene` 相关 shader。

## 2. 当前使用方
- 已注册场景：`SSAOScene`
- 未注册但仍复用同名 shader 路径：`DisneyScene`
- 对应配置名：
  - `ssao_geometry`
  - `ssao`

## 3. 功能需求
- 应覆盖 G-buffer geometry pass 和 SSAO compute/post-process 相关 shader。
- 文件命名应体现 geometry 与 SSAO 阶段差异。
