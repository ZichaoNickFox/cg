#include "engine/mesh.h"

#include "glm/gtx/intersect.hpp"
#include "glm/gtx/string_cast.hpp"
#include <limits>

#include "engine/debug.h"
#include "engine/gl.h"

namespace engine {
namespace {
// TODO : same as playground::util::VectorByteSize
template<typename ElemType>
int VectorSizeInByte(const std::vector<ElemType>& v) {
  return sizeof(ElemType) * v.size();
}
template<typename Type>
void* AsVoidPtr(Type& var) {
  return reinterpret_cast<void*>(var);
}
}

void Mesh::Setup() {
  int enabled_component_num = 0;
  int vertex_size_in_float = 0;
  vertex_size_in_float += kMeshVertexLayout.at("position").size_in_float; enabled_component_num++;
  vertex_size_in_float += kMeshVertexLayout.at("normal").size_in_float; enabled_component_num++;
  vertex_size_in_float += kMeshVertexLayout.at("texcoord").size_in_float; enabled_component_num++;
  vertex_size_in_float += kMeshVertexLayout.at("tangent").size_in_float; enabled_component_num++;
  vertex_size_in_float += kMeshVertexLayout.at("bitangent").size_in_float; enabled_component_num++;

  SetupVBO(vertex_size_in_float);
  SetupVAO(enabled_component_num);
  SetupEBO();
}

void Mesh::SetupVBO(int buffer_size_in_float) {
  int vertex_num = positions_.size();
  CGCHECK(vertex_num  > 0) << " Vertex size must > 0 : mesh=" << name_;

  std::vector<float> buffer(buffer_size_in_float * vertex_num, 0);
  glGenBuffers(1, &vbo_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, VectorSizeInByte(buffer), buffer.data(), GL_STATIC_DRAW);
  int offset_in_vbo = 0; 

#define ADD_SUB_DATA(component) \
  if (component.size() == 0) { \
    component.resize(vertex_num); \
  } else { \
    CGCHECK(component.size() == vertex_num) << component.size() << " " << vertex_num; \
  } \
  glBufferSubData(GL_ARRAY_BUFFER, offset_in_vbo, VectorSizeInByte(component), component.data()); \
  offset_in_vbo += VectorSizeInByte(component);

  ADD_SUB_DATA(positions_);
  ADD_SUB_DATA(normals_);
  ADD_SUB_DATA(texcoords_);
  ADD_SUB_DATA(tangents_);
  ADD_SUB_DATA(bitangents_);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::SetupVAO(int enabled_component_num) {
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);

  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);

  for (int i = 0; i < enabled_component_num; ++i) {
    glEnableVertexAttribArray(i);
  }
  int offset_in_vbo = 0;
  int vertex_component_index = 0;
#define ENABLE_VERTEX_COMPONENT(component, component_index, size_in_float) \
  glVertexAttribPointer(component_index, size_in_float, GL_FLOAT, GL_FALSE, size_in_float * sizeof(float), \
      AsVoidPtr(offset_in_vbo)); \
  offset_in_vbo += VectorSizeInByte(component); \
  vertex_component_index++;

  ENABLE_VERTEX_COMPONENT(positions_, vertex_component_index, kMeshVertexLayout.at("position").size_in_float);
  ENABLE_VERTEX_COMPONENT(normals_, vertex_component_index, kMeshVertexLayout.at("normal").size_in_float);
  ENABLE_VERTEX_COMPONENT(texcoords_, vertex_component_index, kMeshVertexLayout.at("texcoord").size_in_float);
  ENABLE_VERTEX_COMPONENT(tangents_, vertex_component_index, kMeshVertexLayout.at("tangent").size_in_float);
  ENABLE_VERTEX_COMPONENT(bitangents_, vertex_component_index, kMeshVertexLayout.at("bitangent").size_in_float);
  
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::SetupEBO() {
  glBindVertexArray(vao_);

  glGenBuffers(1, &ebo_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, VectorSizeInByte(indices_), indices_.data(), GL_STATIC_DRAW);

  glBindVertexArray(0);
}

void Mesh::Submit() const {
  bool use_ebo = indices_.size() > 0;
  glBindVertexArray(vao_);
  if (use_ebo) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  } else {
    glDrawArrays(GL_TRIANGLES, 0, positions_.size());
  }
  glBindVertexArray(0);
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