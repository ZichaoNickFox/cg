# renderer/shader/bxdf 设计

## 1. 组成
- `brdf.glsl`
- `disney.glsl`

## 2. 设计要点
- 目录按材质模型或 BRDF 家族拆分，而不是按场景拆分。
- 这是纯公式库目录，设计上更接近“shader include 库”而不是“可独立编译的顶层 shader”。
