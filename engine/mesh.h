#pragma once

#include <unordered_map>

#include "glm/glm.hpp"
#include <string>
#include <vector>
#include <memory>

#include "engine/gl.h"
#include "engine/util.h"

namespace engine {
constexpr char kVertexAttributePosition[] = "position";
constexpr char kVertexAttributeNormal[] = "normal";
constexpr char kVertexAttributeTexcoord[] = "texcoord";
constexpr char kVertexAttributeTangent[] = "tangent";
constexpr char kVertexAttributeBitangent[] = "bitangent";
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

// Like positon ~ vec3. model ~ 4 vec4s etc.
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
};

class Mesh {
 public:
  void SetPositions(const std::vector<glm::vec3> &positions) { positions_ = positions; }
  void SetNormals(const std::vector<glm::vec3> &normals) { normals_ = normals; }
  void SetTexcoords(const std::vector<glm::vec2> &texcoords) { texcoords_ = texcoords; }
  void SetTangent(const std::vector<glm::vec3> &tangents) { tangents_ = tangents; }
  void SetBitangent(const std::vector<glm::vec3> &bitangents) { bitangents_ = bitangents; }

  const std::vector<glm::vec3>& positions() const { return positions_; }
  const std::vector<glm::vec3>& normals() const { return normals_; }
  const std::vector<glm::vec2>& texcoords() const { return texcoords_; }
  const std::vector<glm::vec3>& tangents() const { return tangents_; }
  const std::vector<glm::vec3>& bitangents() const { return bitangents_; }

  void SetName(const std::string& name) { name_ = name; }
  const std::string& name() const { return name_; }

  void SetIndices(const std::vector<GLuint> &indices) { indices_ = indices; }
  const std::vector<GLuint>& indices() const { return indices_; }

  void Setup();
  void Submit(int instance_num = 1) const;

  bool Intersect(const glm::vec3& origin_ls, const glm::vec3& dir_ls,
                 glm::vec3* position_ls, glm::vec3* normal_ls, float* distance_ls,
                 glm::vec3* vertex0_ls, glm::vec3* vertex1_ls, glm::vec3* vertex2_ls) const;

  template<typename ElementType>
  void AddVertexAttribute(const VertexAttribute& meta, const std::vector<ElementType>& data);

 private:
  GLuint vao_;
  std::vector<GLuint> vbos_;
  GLuint ebo_;
  std::vector<GLuint> indices_;

  std::vector<glm::vec3> positions_;
  std::vector<glm::vec3> normals_;
  std::vector<glm::vec2> texcoords_;
  std::vector<glm::vec3> tangents_;
  std::vector<glm::vec3> bitangents_;

  std::string name_;
};

template<typename ElementType>
void Mesh::AddVertexAttribute(const VertexAttribute& meta, const std::vector<ElementType>& data) {
  // vbo
  GLuint vbo;
  glGenBuffers_(1, &vbo);
  glBindBuffer_(GL_ARRAY_BUFFER, vbo);
  glBufferData_(GL_ARRAY_BUFFER, sizeof(ElementType) * data.size(), util::AsVoidPtr(data.data()), GL_STATIC_DRAW);
  vbos_.push_back(vbo);

  // vao
  glBindVertexArray_(vao_);
  int layout_index_num = meta.attribute_layout_index_to - meta.atrribute_layout_index_from + 1;
  for (int layout_index = meta.atrribute_layout_index_from, i = 0; layout_index <= meta.attribute_layout_index_to;
      ++layout_index, ++i) {
    glEnableVertexAttribArray_(layout_index);
    int attribute_size_in_byte = meta.attribute_component_num * sizeof(GL_FLOAT);
    int stride = layout_index_num * attribute_size_in_byte;
    glVertexAttribPointer_(layout_index, meta.attribute_component_num, GL_FLOAT, GL_FALSE,
                           stride, util::AsVoidPtr(static_cast<uint64_t>(i * attribute_size_in_byte)));
    if (meta.divisor > 0) {
      glVertexAttribDivisor_(layout_index, meta.divisor);
    }
  }

  // ebo
  if (indices_.size() > 0) {
    glGenBuffers_(1, &ebo_);
    glBindBuffer_(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData_(GL_ELEMENT_ARRAY_BUFFER, util::VectorSizeInByte(indices_), indices_.data(), GL_STATIC_DRAW);
  }

  glBindVertexArray_(0);
  glBindBuffer_(GL_ARRAY_BUFFER, 0);
}
} // namespace engine