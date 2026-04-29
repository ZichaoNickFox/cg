# test 设计

## 1. 角色
`test/` 是比 `playground/test/` 更底层、更细粒度的单元测试目录。

## 2. 设计要点
- 顶层 `CMakeLists.txt` 只在 `BUILD_TESTING` 打开时启用本目录。
- 各子目录按被测模块拆分。
- `test_support/` 抽出 fake 实现，减少 renderer/RHI 测试重复造轮子。
