# playground/shader/rtrt 需求

## 1. 目标
本目录保存 `RTRTScene` 使用的 raster + compute 混合路径 shader。

## 2. 当前使用方
- 已注册场景：`RTRTScene`
- 对应配置名：
  - `rtrt_geometry`
  - `rtrt_path_tracing`
  - `rtrt_outlier_clamping`
  - `rtrt_denoise`
  - `rtrt_temproal_accumulation`

## 3. 功能需求
- 应覆盖 geometry、path tracing、outlier clamping、denoise、temporal accumulation 等阶段。
- 文件命名应与各 pass 一一对应。
- 配置名中的 `temproal` 拼写当前属于兼容现状的一部分，修改时必须同步代码和配置。
