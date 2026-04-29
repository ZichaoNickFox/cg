# playground/test 需求

## 1. 目标
`playground/test/` 目录负责覆盖更接近真实运行时的测试，包括 renderer 行为验证和运行时 smoke test。

## 2. 功能需求
- 应提供与 playground 资源和 renderer 行为相关的单元测试。
- 应提供 `runtime_smoke.py` 驱动的运行时 smoke test。
- smoke test 应能通过环境变量控制运行时并检查关键日志 marker。
- 与图形运行时相关的 smoke test 应串行运行。

## 3. 验收要点
- `playground/test/CMakeLists.txt` 中的测试目标可被 `ctest` 发现。
