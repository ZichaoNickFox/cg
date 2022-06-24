#include "renderer/meshes/lines_mesh.h"

namespace renderer {

LinesMesh::LinesMesh(const std::vector<Triangle>& triangles, const glm::vec4& color) {
  colors_.assign(6 * triangles.size(), color);
  positions_.resize(6 * triangles.size());
  int index = 0;
  for (int i = 0; i < triangles.size(); ++i) {
    positions_[index++] = glm::vec4(triangles[i].a, 1.0);
    positions_[index++] = glm::vec4(triangles[i].b, 1.0);
    positions_[index++] = glm::vec4(triangles[i].b, 1.0);
    positions_[index++] = glm::vec4(triangles[i].c, 1.0);
    positions_[index++] = glm::vec4(triangles[i].c, 1.0);
    positions_[index++] = glm::vec4(triangles[i].a, 1.0);
  }
  primitive_mode_ = GL_LINES;
  Setup();
}

LinesMesh::LinesMesh(const std::vector<AABB>& aabbs, const glm::vec4& force_color) {
  positions_.resize(aabbs.size() * 24);
  colors_.resize(positions_.size());
  if (force_color == glm::vec4(-1, -1, -1, -1)) {
#if CGDEBUG
    for (int i = 0; i < colors_.size(); ++i) {
      colors_[i] = aabbs[i / 24].debug_color;
    }
#else
    colors_ = std::vector<glm::vec4>(colors_.size(), glm::vec4(0, 0, 0, 1));
#endif
  } else {
    colors_ = std::vector<glm::vec4>(colors_.size(), force_color);
  }
  int i = 0;
  for (const renderer::AABB& aabb : aabbs) {
    glm::vec4 a{aabb.minimum.x, aabb.minimum.y, aabb.minimum.z, 1.0};
    glm::vec4 b{aabb.minimum.x, aabb.maximum.y, aabb.minimum.z, 1.0};
    glm::vec4 c{aabb.minimum.x, aabb.maximum.y, aabb.maximum.z, 1.0};
    glm::vec4 d{aabb.minimum.x, aabb.minimum.y, aabb.maximum.z, 1.0};
    glm::vec4 e{aabb.maximum.x, aabb.minimum.y, aabb.minimum.z, 1.0};
    glm::vec4 f{aabb.maximum.x, aabb.maximum.y, aabb.minimum.z, 1.0};
    glm::vec4 g{aabb.maximum.x, aabb.maximum.y, aabb.maximum.z, 1.0};
    glm::vec4 h{aabb.maximum.x, aabb.minimum.y, aabb.maximum.z, 1.0};
    positions_[i++] = a;  positions_[i++] = b;
    positions_[i++] = b;  positions_[i++] = c;
    positions_[i++] = c;  positions_[i++] = d;
    positions_[i++] = d;  positions_[i++] = a;
    positions_[i++] = a;  positions_[i++] = e;
    positions_[i++] = b;  positions_[i++] = f;
    positions_[i++] = c;  positions_[i++] = g;
    positions_[i++] = d;  positions_[i++] = h;
    positions_[i++] = e;  positions_[i++] = f;
    positions_[i++] = f;  positions_[i++] = g;
    positions_[i++] = g;  positions_[i++] = h;
    positions_[i++] = h;  positions_[i++] = e;
  }
  primitive_mode_ = GL_LINES;
  Setup();
}

LinesMesh::LinesMesh(const std::vector<glm::vec3>& positions, const std::vector<glm::vec4>& colors,
                     GLuint primitive_mode) {
  SetPositions(positions);
  SetColors(colors);
  primitive_mode_ = primitive_mode;
  Setup();
}

LinesMesh::LinesMesh(const std::vector<glm::vec4>& positions, const std::vector<glm::vec4>& colors,
                     GLuint primitive_mode) {
  for (const glm::vec4& position : positions) {
    positions_.push_back(position);
  }
  colors_ = colors;
  primitive_mode_ = primitive_mode;
  Setup();
}

} // namespace renderer