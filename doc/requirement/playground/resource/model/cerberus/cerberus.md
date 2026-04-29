# playground/resource/model/cerberus 需求

## 1. 目标
本目录保存 Cerberus 资产集，当前是正式场景路径中的在用模型资源。

## 2. 当前使用方
- 已注册场景：
  - `ModelScene`
  - `SSAOScene`

## 3. 功能需求
- 应包含主体模型文件和配套材质纹理目录。
- 文件命名应保持与外部资源集一致，避免模型引用断裂。
- `playground/config.pb.txt` 中的 `cerberus` 模型项应能通过本目录完整解析出模型与纹理。
