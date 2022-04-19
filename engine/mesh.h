#pragma once

#include <unordered_map>

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include <string>
#include <vector>
#include <memory>

#include "engine/pass.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "engine/material.h"

namespace engine {
struct MeshVertexComponent {
  std::string name;
  GLuint format;
  GLuint type;
  GLuint internal_type;
  uint32_t size_in_float;
  uint32_t size_in_byte;
};
static const std::unordered_map<std::string, MeshVertexComponent> kMeshVertexLayout = {
  {"position", {"position", GL_RGB, GL_FLOAT, GL_RGB32F, 3, 12}},
  {"normal", {"normal", GL_RGB, GL_FLOAT, GL_RGB32F, 3, 12}},
  {"texcoord", {"texcoord", GL_RG, GL_FLOAT, GL_RG32F, 2, 8}},
  {"tangent", {"tangent", GL_RGB, GL_FLOAT, GL_RGB32F, 3, 12}},
  {"bitangent", {"bitangent", GL_RGB, GL_FLOAT, GL_RGB32F, 3, 12}},
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
  void Submit() const;

  bool Intersect(const glm::vec3& origin_ls, const glm::vec3& dir_ls,
                 glm::vec3* position_ls, glm::vec3* normal_ls, float* distance_ls,
                 glm::vec3* vertex0_ls, glm::vec3* vertex1_ls, glm::vec3* vertex2_ls) const;
 private:
  void SetupVBO(int vertex_float_size);
  void SetupVAO(int enabled_component_num);
  void SetupEBO();

  GLuint vao_, vbo_, ebo_;
  std::vector<glm::vec3> positions_;
  std::vector<glm::vec3> normals_;
  std::vector<glm::vec2> texcoords_;
  std::vector<glm::vec3> tangents_;
  std::vector<glm::vec3> bitangents_;

  std::vector<GLuint> indices_;

  std::string name_;
};
} // namespace engine