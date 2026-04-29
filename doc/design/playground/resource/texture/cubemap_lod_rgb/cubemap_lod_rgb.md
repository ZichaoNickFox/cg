# playground/resource/texture/cubemap_lod_rgb 设计

## 1. 角色
这是一个测试数据目录，用于验证 cubemap mip/face 读取是否正确。

## 2. 设计要点
- 每一级都包含 back/front/left/right/top/bottom 六面图片。
- 当前主要服务测试和未注册的 `TextureLodScene`，不属于正式场景目录的核心美术资源。
