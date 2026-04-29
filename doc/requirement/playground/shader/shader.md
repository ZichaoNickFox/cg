# playground/shader 需求

## 1. 目标
`playground/shader/` 保存 playground 私有 GLSL 源码，覆盖 sample、shadow、SSAO、RTRT 等场景。

## 2. 当前使用方式
- 一部分 shader 通过 `playground/config.pb.txt` 注册为正式配置名并服务已注册场景
- 一部分 shader 服务未注册但仍保留源码的实验路径
- 目录中的子目录按场景主题而不是按 shader stage 组织

## 3. 功能需求
- 应按场景主题组织 playground 侧 shader。
- shader 文件命名应能和场景或用途对应，便于在 `config.pb.txt` 或场景代码中引用。
- 目录中的 shader 应仅承载 playground 级场景特化逻辑，不替代 `renderer/shader` 中的通用库。

## 3. 验收要点
- 已注册场景使用到的 playground 私有 shader 在本目录中可定位。
