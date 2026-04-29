# test/base 设计

## 1. 组成
- `util_test.cc`
- `math_test.cc`
- `geometry_test.cc`

## 2. 设计要点
- 每个测试目标与 `cg::base` 直接链接，不依赖 playground 层。
- 该目录是整个测试树中最“纯算法/纯工具”的一层。
