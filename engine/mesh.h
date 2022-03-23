#pragma once

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
  void SetPositions(const std::vector<glm::vec3> &positions);
  void SetNormals(const std::vector<glm::vec3> &normals);
  void SetUvs(const std::vector<glm::vec2> &uvs);
  void SetIndices(const std::vector<GLuint> &indices);
  void Setup();
  void Submit();
  
 private:
  GLuint vao_, vbo_, ebo_;
  std::vector<glm::vec3> positions_;
  std::vector<glm::vec3> normals_;
  std::vector<glm::vec2> uvs_;
  std::vector<GLuint> indices_;
};
} // namespace engine