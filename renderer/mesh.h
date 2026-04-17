#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "glm/glm.hpp"

#include "renderer/primitive.h"
#include "base/util.h"
#include "renderer/transform.h"
#include "rhi/device.h"
#include "rhi/types.h"

namespace cg {
constexpr char kVertexAttributePosition[] = "position";
constexpr char kVertexAttributeNormal[] = "normal";
constexpr char kVertexAttributeTexcoord[] = "texcoord";
constexpr char kVertexAttributeTangent[] = "tangent";
constexpr char kVertexAttributeBitangent[] = "bitangent";
constexpr char kVertexAttributeColor[] = "color";
constexpr int kVertexLayoutIndex0 = 0;
constexpr int kVertexLayoutIndex1 = 1;
constexpr int kVertexLayoutIndex2 = 2;
constexpr int kVertexLayoutIndex3 = 3;
constexpr int kVertexLayoutIndex4 = 4;
constexpr int kVertexLayoutIndex5 = 5;
constexpr int kVertexLayoutIndex6 = 6;
constexpr int kVertexLayoutIndex7 = 7;
constexpr int kVertexLayoutIndex8 = 8;
constexpr int kVertexLayoutIndex9 = 9;
constexpr int kVertexLayoutIndex10 = 10;
constexpr int kVertexLayoutIndex11 = 11;
constexpr int kVertexLayoutIndex12 = 12;

struct VertexAttribute {
  std::string name;
  int atrribute_layout_index_from;
  int attribute_layout_index_to;
  int attribute_component_num; // 1 2 3 4
  int divisor = 0;  // instanced divisor
};
static const std::unordered_map<std::string, VertexAttribute> kMeshVertexLayout = {
  {"position", {"position", kVertexLayoutIndex0, kVertexLayoutIndex0, 3, 0}},
  {"normal", {"normal", kVertexLayoutIndex1, kVertexLayoutIndex1, 3, 0}},
  {"texcoord", {"texcoord", kVertexLayoutIndex2, kVertexLayoutIndex2, 2, 0}},
  {"tangent", {"tangent", kVertexLayoutIndex3, kVertexLayoutIndex3, 3, 0}},
  {"bitangent", {"bitangent", kVertexLayoutIndex4, kVertexLayoutIndex4, 3, 0}},
  {"color", {"color", kVertexLayoutIndex5, kVertexLayoutIndex5, 4, 0}}
};

class Mesh {
 public:
  Mesh();
  ~Mesh();

  void SetPositions(const std::vector<glm::vec3>& positions) { positions_ = positions; MarkGpuUploadDirty(); }
  void SetNormals(const std::vector<glm::vec3>& normals) { normals_ = normals; MarkGpuUploadDirty(); }
  void SetTexcoords(const std::vector<glm::vec2>& texcoords) { texcoords_ = texcoords; MarkGpuUploadDirty(); }
  void SetTangent(const std::vector<glm::vec3>& tangents) { tangents_ = tangents; MarkGpuUploadDirty(); }
  void SetBitangent(const std::vector<glm::vec3>& bitangents) { bitangents_ = bitangents; MarkGpuUploadDirty(); }
  void SetColors(const std::vector<glm::vec4>& colors) { colors_ = colors; MarkGpuUploadDirty(); }

  const std::vector<glm::vec3>& positions() const { return positions_; }
  const std::vector<glm::vec3>& normals() const { return normals_; }
  const std::vector<glm::vec2>& texcoords() const { return texcoords_; }
  const std::vector<glm::vec3>& tangents() const { return tangents_; }
  const std::vector<glm::vec3>& bitangents() const { return bitangents_; }
  const std::vector<glm::vec4>& colors() const { return colors_; }

  std::vector<glm::vec3>* mutable_positions() { MarkGpuUploadDirty(); return &positions_; }
  std::vector<glm::vec3>* mutable_normals() { MarkGpuUploadDirty(); return &normals_; }
  std::vector<glm::vec2>* mutable_texcoords() { MarkGpuUploadDirty(); return &texcoords_; }
  std::vector<glm::vec3>* mutable_tangents() { MarkGpuUploadDirty(); return &tangents_; }
  std::vector<glm::vec3>* mutable_bitangents() { MarkGpuUploadDirty(); return &bitangents_; }
  std::vector<glm::vec4>* mutable_colors() { MarkGpuUploadDirty(); return &colors_; }

  void SetName(const std::string& name) { name_ = name; }
  const std::string& name() const { return name_; }

  void SetIndices(const std::vector<uint32_t>& indices) { indices_ = indices; MarkGpuUploadDirty(); }
  const std::vector<uint32_t>& indices() const { return indices_; }
  std::vector<uint32_t>* mutable_indices() { MarkGpuUploadDirty(); return &indices_; }

  void Setup();
  void Submit(int instance_num = 1) const;

  bool Intersect(const glm::vec3& origin_ls, const glm::vec3& dir_ls,
                 glm::vec3* position_ls, glm::vec3* normal_ls, float* distance_ls,
                 glm::vec3* vertex0_ls, glm::vec3* vertex1_ls, glm::vec3* vertex2_ls) const;

  int BreakIntoPrimitives(int material_index, const Transform& transform, PrimitiveRepo* primitive_repo) const;

 protected:
  std::vector<glm::vec3> positions_;
  std::vector<glm::vec3> normals_;
  std::vector<glm::vec2> texcoords_;
  std::vector<glm::vec3> tangents_;
  std::vector<glm::vec3> bitangents_;
  std::vector<glm::vec4> colors_;
  std::vector<uint32_t> indices_;

  std::string name_;

  rhi::PrimitiveTopology primitive_mode_ = rhi::PrimitiveTopology::kTriangles;

 private:
  void MarkGpuUploadDirty() const;
  void EnsureGpuResourcesReady() const;
  void ResetGpuResources() const;

  template<typename ElementType>
  void UploadVertexAttribute(const VertexAttribute& meta, const std::vector<ElementType>& data) const;

  mutable std::unique_ptr<rhi::VertexArray> vao_;
  mutable std::vector<std::unique_ptr<rhi::Buffer>> vbos_;
  mutable std::unique_ptr<rhi::Buffer> ebo_;
  mutable bool gpu_upload_dirty_ = true;
};

class MeshRepo {
 public:
  MeshRepo();
  bool Has(const std::string& mesh_name) const { return name_2_index_.find(mesh_name) != name_2_index_.end(); }
  void Add(const std::string& mesh_name, std::unique_ptr<Mesh> mesh);
  int BreakIntoPrimitives(int mesh_index, int material_index, const Transform& transform,
                         PrimitiveRepo* primitive_repo) const;
  const Mesh* GetMesh(int mesh_index) const;
  int GetIndex(const std::string& mesh_name) const;
  std::string GetName(int mesh_index) const;

 private:
  std::unordered_map<int, std::unique_ptr<Mesh>> index_2_mesh_;
  std::unordered_map<std::string, int> name_2_index_;
};
} // namespace cg
