# playground/resource/model/cerberus 设计

## 1. 组成
- `Cerberus_LP.FBX`
- `Textures/`

## 2. 设计要点
- 模型与纹理分目录放置，便于保留原始资源集结构。
- 当前 `ModelScene` 和 `SSAOScene` 通过配置名 `cerberus` 间接引用该目录。
- 这是“正式注册场景正在使用”的复杂模型目录，优先保证稳定性。
