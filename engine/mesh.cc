#include "engine/mesh.h"

#include "glm/gtx/intersect.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <limits>

#include "engine/debug.h"
#include "engine/gl.h"

namespace engine {
void Mesh::Setup() {
  glGenVertexArrays_(1, &vao_);
  AddVertexAttribute(kMeshVertexLayout.at(kVertexAttributePosition), positions_);
  AddVertexAttribute(kMeshVertexLayout.at(kVertexAttributeNormal), normals_);
  AddVertexAttribute(kMeshVertexLayout.at(kVertexAttributeTexcoord), texcoords_);
  AddVertexAttribute(kMeshVertexLayout.at(kVertexAttributeTangent), tangents_);
  AddVertexAttribute(kMeshVertexLayout.at(kVertexAttributeBitangent), bitangents_);
}

void Mesh::Submit(int instance_num) const {
  bool use_ebo = indices_.size() > 0;
  glBindVertexArray_(vao_);
  if (use_ebo) {
    glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    if (instance_num > 0) {
      glDrawElementsInstanced_(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0, instance_num);
    } else {
      glDrawElements_(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
    }
    glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, 0);
  } else {
    if (instance_num > 0) {
      glDrawArraysInstanced_(GL_TRIANGLES, 0, positions_.size(), instance_num);
    } else {
      glDrawArrays_(GL_TRIANGLES, 0, positions_.size());
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
}