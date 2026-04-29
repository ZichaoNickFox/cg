# playground/resource 设计

## 1. 角色
本目录是 playground 运行时资产根目录。

## 2. 分层
- `model/`：OBJ、FBX 及其配套材质/纹理
- `texture/`：2D 纹理、cubemap、IBL 贴图和预计算结果
- `shadow_scene/`：场景专属说明

## 3. 设计要点
- 资源目录按“资产类型”优先组织，再按具体主题细分。
- 资产本身不包含业务逻辑，实际引用关系由 `Config`、场景代码和模型加载器决定。
