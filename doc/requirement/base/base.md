# base 需求

## 1. 目标
`base/` 目录向上层提供通用基础能力，作为 geometry、renderer、playground、test 等模块共享的底座。

## 2. 范围
- 调试与断言支持
- 文件/字符串/时间/STL 辅助函数
- 基础数学与采样函数
- CPU 侧几何数据结构与相交计算
- 常用颜色常量

## 3. 功能需求
- 应提供不依赖窗口系统的纯基础能力。
- `util` 应支持文件读取、路径拼接、格式化、时间计算与常用容器转换。
- `math` 应支持插值、裁剪、采样、噪声、随机数、`QuickSelect`、`Linspace` 等通用能力。
- `geometry` 应提供 `Ray`、`Sphere`、`AABB`、`Triangle` 等结构以及 CPU 侧几何运算支撑。
- 该目录中的 API 应可被测试独立验证。

## 4. 依赖与约束
- 可依赖 `glog`、`glm`、`Eigen`、`fmt` 等通用库。
- 不应依赖 `GLFW`、`ImGui` 或具体图形后端。

## 5. 验收要点
- `test/base` 中的测试应覆盖本目录主要能力。
