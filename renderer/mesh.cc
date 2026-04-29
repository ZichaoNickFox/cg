#include "renderer/mesh.h"

#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/intersect.hpp"
#include "glm/gtx/string_cast.hpp"

#include "base/debug.h"
#include "renderer/mesh/cube_mesh.h"
#include "renderer/mesh/lines_mesh.h"
#include "renderer/mesh/sphere_mesh.h"
#include "renderer/object.h"
#include "renderer/mesh/plane_mesh.h"
#include "rhi/device.h"

namespace cg {
template<typename ElementType>
void Mesh::UploadVertexAttribute(const VertexAttribute& meta, const std::vector<ElementType>& data) const {
  if (data.empty()) {
    return;
  }

  auto vbo = rhi::GetDevice().CreateBuffer(rhi::BufferType::kVertex);
  vbo->SetData(util::VectorSizeInByte(data), util::AsVoidPtr(data.data()), rhi::BufferUsage::kStatic);

  rhi::VertexArrayBindingDesc binding_desc;
  binding_desc.buffer = vbo.get();
  int layout_index_num = meta.attribute_layout_index_to - meta.atrribute_layout_index_from + 1;
  for (int layout_index = meta.atrribute_layout_index_from, i = 0; layout_index <= meta.attribute_layout_index_to;
       ++layout_index, ++i) {
    int attribute_size_in_byte = meta.attribute_component_num * sizeof(float);
    int stride = layout_index_num * attribute_size_in_byte;
    binding_desc.attributes.push_back({
        .index = static_cast<uint32_t>(layout_index),
        .component_count = meta.attribute_component_num,
        .stride_in_bytes = stride,
        .offset_in_bytes = static_cast<size_t>(i * attribute_size_in_byte),
        .divisor = static_cast<uint32_t>(meta.divisor),
    });
  }
  vao_->ApplyBinding(binding_desc);
  vbos_.push_back(std::move(vbo));
}

template void Mesh::UploadVertexAttribute<glm::vec2>(const VertexAttribute& meta,
                                                     const std::vector<glm::vec2>& data) const;
template void Mesh::UploadVertexAttribute<glm::vec3>(const VertexAttribute& meta,
                                                     const std::vector<glm::vec3>& data) const;
template void Mesh::UploadVertexAttribute<glm::vec4>(const VertexAttribute& meta,
                                                     const std::vector<glm::vec4>& data) const;

Mesh::Mesh() = default;

Mesh::~Mesh() {
  if (!rhi::HasDevice()) {
    vao_.release();
    ebo_.release();
    for (auto& vbo : vbos_) {
      vbo.release();
    }
    vbos_.clear();
    return;
  }
  ResetGpuResources();
}

void Mesh::MarkGpuUploadDirty() const {
  gpu_upload_dirty_ = true;
}

void Mesh::Setup() {
  MarkGpuUploadDirty();
}

void Mesh::EnsureGpuResourcesReady() const {
  CGCHECK(rhi::HasDevice()) << "Mesh submission requires an active RHI device.";
  if (!gpu_upload_dirty_ && vao_ != nullptr) {
    return;
  }

  ResetGpuResources();

  vao_ = rhi::GetDevice().CreateVertexArray();
  UploadVertexAttribute(kMeshVertexLayout.at(kVertexAttributePosition), positions_);
  UploadVertexAttribute(kMeshVertexLayout.at(kVertexAttributeNormal), normals_);
  UploadVertexAttribute(kMeshVertexLayout.at(kVertexAttributeTexcoord), texcoords_);
  UploadVertexAttribute(kMeshVertexLayout.at(kVertexAttributeTangent), tangents_);
  UploadVertexAttribute(kMeshVertexLayout.at(kVertexAttributeBitangent), bitangents_);
  UploadVertexAttribute(kMeshVertexLayout.at(kVertexAttributeColor), colors_);

  if (!indices_.empty()) {
    ebo_ = rhi::GetDevice().CreateBuffer(rhi::BufferType::kIndex);
    ebo_->SetData(util::VectorSizeInByte(indices_), indices_.data(), rhi::BufferUsage::kStatic);
  }
  gpu_upload_dirty_ = false;
}

void Mesh::ResetGpuResources() const {
  if (!rhi::HasDevice()) {
    vao_.release();
    ebo_.release();
    for (auto& vbo : vbos_) {
      vbo.release();
    }
    vbos_.clear();
    return;
  }

  vbos_.clear();
  ebo_.reset();
  vao_.reset();
}

rhi::DrawDesc Mesh::BuildDrawDesc(int instance_num) const {
  EnsureGpuResourcesReady();
  bool use_ebo = indices_.size() > 0;
  CGCHECK(vao_ != nullptr) << "vertex array not initialized";
  if (use_ebo) {
    CGCHECK(ebo_ != nullptr) << "index buffer not initialized";
    return {
        .kind = rhi::DrawKind::kElements,
        .topology = primitive_mode_,
        .vertex_array = vao_.get(),
        .index_buffer = ebo_.get(),
        .count = static_cast<uint32_t>(indices_.size()),
        .instance_count = static_cast<uint32_t>(instance_num),
    };
  }
  return {
      .kind = rhi::DrawKind::kArrays,
      .topology = primitive_mode_,
      .vertex_array = vao_.get(),
      .first = 0,
      .count = static_cast<uint32_t>(positions_.size()),
      .instance_count = static_cast<uint32_t>(instance_num),
  };
}

void Mesh::Submit(int instance_num) const {
  rhi::GetDevice().Draw(BuildDrawDesc(instance_num));
}

bool Mesh::Intersect(const glm::vec3& origin_ls, const glm::vec3& dir_ls,
                     glm::vec3* position_ls, glm::vec3* normal_ls, float* distance_ls,
                     glm::vec3* vertex0_ls, glm::vec3* vertex1_ls, glm::vec3* vertex2_ls) const {
  float min_distance = std::numeric_limits<float>::max();
  int found_index = -1;
  for (int i = 0; i < positions_.size(); i += 3) {
    const glm::vec3& vertex0 = positions_[i];
    const glm::vec3& vertex1 = positions_[i + 1];
    const glm::vec3& vertex2 = positions_[i + 2];
    glm::vec2 bary_position;
    float iter_distance;
    if (glm::intersectRayTriangle(origin_ls, glm::normalize(dir_ls), vertex0, vertex1, vertex2,
                                  bary_position, iter_distance)) {
      if (min_distance > iter_distance) {
        found_index = i;
        min_distance = iter_distance;
        *vertex0_ls = vertex0;
        *vertex1_ls = vertex1;
        *vertex2_ls = vertex2;
        *distance_ls = min_distance;
        *position_ls = origin_ls + min_distance * glm::normalize(dir_ls);
      }
    }
  }
  if (found_index != -1) {
    // 3 normals, use 2 or 3 same direction as target direction
    glm::vec3 normal1 = normals_[found_index];
    glm::vec3 normal2 = normals_[found_index + 1];
    glm::vec3 normal3 = normals_[found_index + 2];
    glm::vec3 triangle_dir = glm::cross(positions_[found_index + 1] - positions_[found_index],
                                        positions_[found_index + 2]- positions_[found_index + 1]);
    glm::vec3 normal_dir = normal1;
    if (glm::dot(normal1, normal2) < 0 && glm::dot(normal1, normal3) < 0) {
      normal_dir = normal2;
    }
    *normal_ls = (glm::dot(normal_dir, triangle_dir) < 0) ? -triangle_dir : triangle_dir;
    *normal_ls = glm::normalize(*normal_ls);
  }
  return (found_index != -1);
}

int Mesh::BreakIntoPrimitives(int material_index, const Transform& transform, PrimitiveRepo* primitive_repo) const {
  std::vector<glm::vec3> world_positions(positions_.size());
  glm::mat4 model = transform.GetModelMatrix();
  for (int i = 0; i < positions_.size(); ++i) {
    world_positions[i] = model * glm::vec4(positions_.at(i), 1.0);
  }
  if (indices_.size() > 0) {
    CGCHECK(indices_.size() % 3 == 0);
    int index = 0;
    for (int i = 0, j = 1, k = 2; k < indices_.size(); i += 3, j += 3, k += 3) {
      int mesh_index_i = indices_.at(i);
      int mesh_index_j = indices_.at(j);
      int mesh_index_k = indices_.at(k);
      Triangle triangle{world_positions[mesh_index_i], world_positions[mesh_index_j], world_positions[mesh_index_k]};
      primitive_repo->PushTriangle(triangle, material_index);
    }
    return indices_.size() / 3;
  } else {
    CGCHECK(positions_.size() % 3 == 0);
    int index = 0;
    for (int i = 0, j = 1, k = 2; k < positions_.size(); i += 3, j += 3, k += 3) {
      Triangle triangle{world_positions[i], world_positions[j], world_positions[k]};
      primitive_repo->PushTriangle(triangle, material_index);
    }
    return positions_.size() / 3;
  }
}

MeshRepo::MeshRepo() {
  CGLOG(ERROR) << "Init Mesh : cube";
  name_2_index_["cube"] = 0;
  index_2_mesh_[0] = std::move(std::make_unique<CubeMesh>());

  CGLOG(ERROR) << "Init Mesh : plane";
  name_2_index_["plane"] = 1;
  index_2_mesh_[1] = std::move(std::make_unique<PlaneMesh>());

  CGLOG(ERROR) << "Init Mesh : sphere";
  name_2_index_["sphere"] = 2;
  index_2_mesh_[2] = std::move(std::make_unique<SphereMesh>());

  CGLOG(ERROR) << "Init Mesh : coordinator";
  name_2_index_["coordinator"] = 3;
  index_2_mesh_[3] = std::move(std::make_unique<CoordinatorMesh>());
}

void MeshRepo::Add(const std::string& mesh_name, std::unique_ptr<Mesh> mesh) {
  CGCHECK(!Has(mesh_name)) << mesh_name;
  int mesh_index = name_2_index_.size();
  name_2_index_[mesh_name] = mesh_index;
  index_2_mesh_[mesh_index] = std::move(mesh);
}

int MeshRepo::BreakIntoPrimitives(int mesh_index, int material_index, const Transform& transform,
                                  PrimitiveRepo* primitive_repo) const {
  return GetMesh(mesh_index)->BreakIntoPrimitives(material_index, transform, primitive_repo);
}

const Mesh* MeshRepo::GetMesh(int mesh_index) const {
  CGCHECK(index_2_mesh_.find(mesh_index) != index_2_mesh_.end());
  return index_2_mesh_.at(mesh_index).get();
}

int MeshRepo::GetIndex(const std::string& mesh_name) const {
  CGCHECK(name_2_index_.find(mesh_name) != name_2_index_.end());
  return name_2_index_.at(mesh_name);
}

std::string MeshRepo::GetName(int mesh_index) const {
  for (const auto& p : name_2_index_) {
    if (p.second == mesh_index) {
      return p.first;
    }
  }
  CGCHECK(false) << "Cannot find mesh name of index : " << mesh_index;
  for (const auto& p : name_2_index_) {
    CGLOG(ERROR) << p.first << " " << p.second;
  }
  return "";
}
} // namespace cg
