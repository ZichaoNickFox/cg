#pragma once

#include <unordered_map>

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include <string>
#include <vector>
#include <memory>

#include "engine/shader.h"
#include "engine/texture.h"
#include "engine/material.h"

namespace engine {
class Mesh {
 public:
  struct VertexComponent {
    std::string name;
    GLuint format;
    GLuint type;
    uint32_t size_in_float;
    uint32_t size_in_byte;
  };
  static std::unordered_map<std::string, Mesh::VertexComponent> kVertexLayout;

  void SetPositions(const std::vector<glm::vec3> &positions) { positions_ = positions; }
  void SetNormals(const std::vector<glm::vec3> &normals) { normals_ = normals; }
  void SetTexcoords(const std::vector<glm::vec2> &texcoords) { texcoords_ = texcoords; }
  void SetColors(const std::vector<glm::vec3>& colors) { colors_ = colors; }
  void SetTangent(const std::vector<glm::vec3> &tangents) { tangents_ = tangents; }
  void SetBitangent(const std::vector<glm::vec3> &bitangents) { bitangents_ = bitangents; }

  const std::vector<glm::vec3>& positions() { return positions_; }
  const std::vector<glm::vec3>& normals() { return normals_; }
  const std::vector<glm::vec2>& texcoords() { return texcoords_; }
  const std::vector<glm::vec3>& tangents() { return tangents_; }
  const std::vector<glm::vec3>& bitangents() { return bitangents_; }
  const std::vector<glm::vec3>& colors() { return colors_; }

  void SetName(const std::string& name) { name_ = name; }
  const std::string& name() { return name_; }

  void SetIndices(const std::vector<GLuint> &indices) { indices_ = indices; }
  const std::vector<GLuint>& indices() { return indices_; }

  void Setup();
  void Submit();
  
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
  std::vector<glm::vec3> colors_;

  std::vector<GLuint> indices_;

  std::string name_;
};
} // namespace engine