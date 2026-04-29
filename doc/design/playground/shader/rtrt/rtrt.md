# playground/shader/rtrt 设计

## 1. 组成
- 几何 pass：`rtrt_geometry.*`
- compute pass：`rtrt_path_tracing.cs.glsl`、`rtrt_outlier_clamping.cs.glsl`、`rtrt_denoise.cs.glsl`、`rtrt_temproal_accumulation.cs.glsl`

## 2. 设计要点
- 目录按 RTRT 管线阶段组织，而不是按 shader 类型分层。
- `RTRTScene` 在代码中按这些 pass 的执行顺序组合整条混合渲染链。
