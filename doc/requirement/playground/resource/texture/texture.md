# playground/resource/texture 需求

## 1. 目标
本目录保存 playground 场景会直接或间接使用的纹理资源，是 `TextureConfig` 的主要资产根目录。

## 2. 当前使用方
- 已注册路径在用：
  - `ShadowScene`、`ModelScene`、`SSAOScene` 等通过 `TextureRepo` 间接读取普通纹理
  - `playground/test/renderer_test.cc` 读取 `brickwall`、`skybox`、`cubemap_lod_rgb`
- 未注册但仍有源码引用：
  - `SkyboxScene` 使用 `pbr_environment_tropical`
  - `PbrScene` 使用 `pbr_environment_tropical`、`pbr_irradiance_tropical`、`pbr_prefiltered_color_tropical`
  - `TextureLodScene` 使用 `texture2d_lod_rgb`、`cubemap_lod_rgb`

## 3. 功能需求
- 应覆盖普通 2D 纹理、skybox/cubemap、equirectangular HDR、PBR IBL 预计算结果等类型。
- 文件布局应支持 `TextureRepo`、`Config` 和场景代码按固定路径读取。
- 多级 mip 或多面 cubemap 数据应以清晰命名表达 level/face 信息。
- 目录中的纹理分类应允许同时服务正式注册场景、测试和未注册实验路径，但文档需区分这些用途。
