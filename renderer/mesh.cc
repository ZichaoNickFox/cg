#include "renderer/mesh.h"

#include "glm/gtx/intersect.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <limits>

#include "renderer/debug.h"
#include "renderer/gl.h"
#include "renderer/mesh/cube_mesh.h"
#include "renderer/mesh/lines_mesh.h"
#include "renderer/mesh/plane_mesh.h"
#include "renderer/mesh/sphere_mesh.h"
#include "renderer/object.h"

namespace renderer {
Mesh::Mesh() {
  glGenVertexArrays_(1, &vao_);
}

Mesh::~Mesh() {
  glDeleteVertexArrays_(1, &vao_);
}

void Mesh::Setup() {
  AddVertexAttribute(kMeshVertexLayout.at(kVertexAttributePosition), positions_);
  AddVertexAttribute(kMeshVertexLayout.at(kVertexAttributeNormal), normals_);
  AddVertexAttribute(kMeshVertexLayout.at(kVertexAttributeTexcoord), texcoords_);
  AddVertexAttribute(kMeshVertexLayout.at(kVertexAttributeTangent), tangents_);
  AddVertexAttribute(kMeshVertexLayout.at(kVertexAttributeBitangent), bitangents_);
  AddVertexAttribute(kMeshVertexLayout.at(kVertexAttributeColor), colors_);

  // ebo
  if (indices_.size() > 0) {
    glGenBuffers_(1, &ebo_);
    glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData_(GL_ELEMENT_ARRAY_BUFFER, util::VectorSizeInByte(indices_), indices_.data(), GL_STATIC_DRAW);
  }
}

void Mesh::Submit(int instance_num) const {
  bool use_ebo = indices_.size() > 0;
  CGCHECK(vao_ != std::numeric_limits<GLuint>::max()) << " vao not set : " << vao_;
  glBindVertexArray_(vao_);
  if (use_ebo) {
    CGCHECK(ebo_ != std::numeric_limits<GLuint>::max()) << " ebo not set : " << ebo_;
    glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    if (instance_num > 1) {
      glDrawElementsInstanced_(primitive_mode_, indices_.size(), GL_UNSIGNED_INT, 0, instance_num);
    } else {
      glDrawElements_(primitive_mode_, indices_.size(), GL_UNSIGNED_INT, 0);
    }
    glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, 0);
  } else {
    if (instance_num > 1) {
      glDrawArraysInstanced_(primitive_mode_, 0, positions_.size(), instance_num);
    } else {
      glDrawArrays_(primitive_mode_, 0, positions_.size());
    }
  }
  glBindVertexArray_(0);
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
}