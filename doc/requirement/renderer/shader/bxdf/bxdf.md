# renderer/shader/bxdf 需求

## 1. 目标
本目录保存 BRDF/BXDF 相关通用公式代码。

## 2. 当前使用方
- 主要由 `renderer/shader/pbr/` 中的 shader include
- 当前不直接作为顶层 `ShaderConfig` 被加载

## 3. 功能需求
- 应提供可被 PBR 或更复杂材质模型 include 的着色函数。
- 公式实现应保持纯 GLSL 库属性，不直接绑定具体场景。
