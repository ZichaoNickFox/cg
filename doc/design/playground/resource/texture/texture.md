# playground/resource/texture 设计

## 1. 分层
- 根目录：普通 2D 纹理与通用图标贴图
- `cubemap_lod_rgb/`：多面多级 cubemap 测试纹理
- `equirectangular/`：HDR/IBL 源环境图
- `pbr_environment_tropical/`：环境 cubemap
- `pbr_irradiance_tropical/`：irradiance cubemap
- `pbr_prefiltered_color_tropical/`：prefiltered cubemap
- `skybox/`：传统 6 面 skybox
- `texture2d_lod_rgb/`：多级 2D 纹理测试集

## 2. 设计要点
- 目录命名紧贴场景实验主题，便于从资源路径反推出用途。
- `playground/config.pb.txt` 是这些纹理子目录进入程序的统一映射点。
- 其中一部分目录偏正式运行时输入，另一部分更偏测试或未注册实验路径输入。
