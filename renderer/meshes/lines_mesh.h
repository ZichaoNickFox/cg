#pragma once

#include <glm/glm.hpp>

#include "renderer/color.h"
#include "renderer/geometry.h"
#include "renderer/mesh.h"

namespace renderer {

class LinesMesh : public Mesh {
 public:
  LinesMesh(const std::vector<Triangle>& triangles, const glm::vec4& color);
  LinesMesh(const std::vector<AABB>& aabbs, const glm::vec4& force_color = glm::vec4(-1, -1, -1, -1));
  LinesMesh(const std::vector<glm::vec3>& positions, const std::vector<glm::vec4>& colors, GLuint primitive_mode);
  LinesMesh(const std::vector<glm::vec4>& positions, const std::vector<glm::vec4>& colors, GLuint primitive_mode);
};

class CoordinatorMesh : public LinesMesh {
 public:
  CoordinatorMesh() : LinesMesh({glm::vec3(0, 0, 0), glm::vec3(1, 0, 0),
                                 glm::vec3(0, 0, 0), glm::vec3(0, 1, 0),
                                 glm::vec3(0, 0, 0), glm::vec3(0, 0, 1)},
                                {kRed, kRed, kGreen, kGreen, kBlue, kBlue}, GL_LINES) {}
};

} // namespace renderer