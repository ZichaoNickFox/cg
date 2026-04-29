# playground/test 设计

## 1. 组成
- `eigen_test.cc`
- `geometry_test.cc`
- `renderer_test.cc`
- `runtime_smoke.py`
- `CMakeLists.txt`

## 2. 设计要点
- 该目录测试比 `test/` 更靠近真实资源和运行时入口。
- `runtime_smoke.py` 通过拉起 `window` 进程并检查日志 marker 验证路径是否走通。
- `CMakeLists.txt` 对多条 runtime smoke tests 设置 `RUN_SERIAL TRUE`，避免窗口和驱动争用。

## 3. 边界
- 像 fake device 这类底层测试支撑不在这里，而在 `test/test_support/`。
