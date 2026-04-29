# cmake 设计

## 1. 角色
`cmake/` 是顶层构建系统的辅助层，当前由三个文件组成：

- `Dependencies.cmake`：统一接入第三方依赖并设置相关开关
- `FindMesaEGL.cmake`：查找 Mesa EGL
- `FindOSMesa.cmake`：查找 OSMesa

## 2. 设计要点
- `Dependencies.cmake` 先校验 `thirdparty/` 中的关键目录，再集中 `add_subdirectory(...)` 引入依赖。
- 该文件同时把外部 target 规范化为 `cg::glm`、`cg::imgui`、`protobuf::libprotobuf` 这类工程内稳定别名。
- `FindMesaEGL.cmake` 和 `FindOSMesa.cmake` 都采用“环境变量 + 常见安装前缀 + pkg-config”的混合查找策略。
- 查找结果通过 `CG_HAS_OSMESA`、`CG_HAS_MESA_EGL`、`CG_HAS_MESA_OFFSCREEN` 这类内部变量暴露给上层。

## 3. 边界
- 这里只负责配置期决策，不负责运行时分流。
- Vulkan 的启用由 `rhi/vulkan/CMakeLists.txt` 继续按 `find_package(Vulkan)` 决定，不在本目录中硬编码。
