# md 设计

## 1. 角色
`md/` 是仓库外显展示资源目录，当前主要内容是：

- 若干场景效果截图
- `reference.md`

## 2. 设计要点
- 文件按“效果主题”而不是模块代码组织。
- 本目录与 `playground/resource/` 区分明确：`md/` 用于文档展示，`playground/resource/` 用于程序运行时加载。

## 3. 边界
- 不在此目录存放 shader、model、texture 的运行时输入资源。
