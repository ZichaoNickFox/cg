# renderer/shader/phong 需求

## 1. 目标
本目录保存传统 Phong 着色 shader。

## 2. 当前使用方
- 顶层配置名：`phong`
- 已注册场景中的 `ModelScene` 会直接消费该路径
- 其他路径如 `SSAO_lighting` 也会复用其中的片元着色逻辑

## 3. 功能需求
- 应支持基础模型照明。
- 公共模型相关逻辑应抽到共享 include 文件中。
