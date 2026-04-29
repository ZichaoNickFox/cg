# renderer/mesh 设计

## 1. 组成
- `cube_mesh.*`
- `plane_mesh.*`
- `sphere_mesh.*`
- `lines_mesh.*`
- `empty_mesh.*`
- `mesh_util.*`

## 2. 设计要点
- `Mesh` 维护 CPU 顶点数据，并在需要时惰性上传 GPU 资源。
- 顶点属性布局通过 `kMeshVertexLayout` 固定约定，减少场景层重复配置 attribute。
- `MeshRepo` 负责名称到网格索引/对象的映射。
