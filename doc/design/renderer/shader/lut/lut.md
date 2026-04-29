# renderer/shader/lut 设计

## 1. 组成
- `distance_lut.glsl/.py`
- `gaussian_lut.glsl/.py`

## 2. 设计要点
- 通过“脚本 + 产物”并存的方式兼顾可追溯性和运行时直接 include 的便利。
- 运行时只需要 `.glsl` 产物；`.py` 的存在是为了让查找表可再生、可审计。
