# playground/resource/model 设计

## 1. 角色
本目录按模型主题拆分子目录，既包含简单单文件模型，也包含带贴图和材质文件的复杂资产。

## 2. 当前布局
- 根目录直放：
  - `bunny.obj`
  - `teapot.obj`
- 主题子目录：
  - `cerberus/`
  - `cornell_box/`
  - `nanosuit/`
  - `planet/`
  - `rock/`

## 3. 设计要点
- “根目录单文件”模式适合简单模型，`model_dir` 可以直接指向 `playground/resource/model`。
- “主题子目录”模式适合带材质和纹理的复杂资源包，`model_dir` 指向具体子目录。
- 该目录的最终入口不是自动扫描，而是 `playground/config.pb.txt` 中的 `ModelConfig`。
