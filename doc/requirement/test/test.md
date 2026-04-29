# test 需求

## 1. 目标
`test/` 目录承载本工程的常规单元测试，用于验证基础算法、renderer 行为和 RHI 抽象。

## 2. 范围
- `base`
- `geometry`
- `physics`
- `renderer`
- `rhi`
- `test_support`

## 3. 功能需求
- 应提供可被 `ctest` 发现的单元测试目标。
- 应覆盖关键基础模块与抽象边界。
- 应通过 `test_support/` 提供可复用的 fake device 等测试支撑。
