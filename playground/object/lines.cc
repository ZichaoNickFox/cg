#include "playground/object/lines.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

#include "engine/util.h"

void Lines::SetMesh(const Mesh& data) {
  Clear();
  OnInit(data);
}

void Lines::OnInit(const Mesh& data) {
  primitive_mode_ = data.primitive_mode;
  vertex_size_ = data.points.size();

  CGCHECK(data.points.size() == data.colors.size());
  std::vector<glm::vec3> buffer(data.points.size() + data.colors.size(), glm::vec3());
  glGenBuffers_(1, &vbo_);
  glBindBuffer_(GL_ARRAY_BUFFER, vbo_);
  glBufferData_(GL_ARRAY_BUFFER, util::VectorByteSize(buffer), buffer.data(), GL_STATIC_DRAW);
  glBufferSubData_(GL_ARRAY_BUFFER, 0, util::VectorByteSize(data.points), data.points.data());
  glBufferSubData_(GL_ARRAY_BUFFER, util::VectorByteSize(data.points), util::VectorByteSize(data.colors), data.colors.data());

  const int kPosLayout = 0;
  const int kColorLayout = 1;
  glGenVertexArrays_(1, &vao_);
  glBindVertexArray_(vao_);
  glEnableVertexAttribArray_(kPosLayout);
  glEnableVertexAttribArray_(kColorLayout);

  // Stride param must assign to vertex size 8 * sizeof(float)
  // Offset param means offset in a vertex
  glVertexAttribPointer_(kPosLayout, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glVertexAttribPointer_(kColorLayout, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                         reinterpret_cast<void *>(util::VectorByteSize(data.points)));

  glBindBuffer_(GL_ARRAY_BUFFER, 0);
  glBindVertexArray_(0);
}

void Lines::OnUpdate(Context *context) {

}

void Lines::OnRender(Context *context) {
  material_.PrepareShader();
  glBindVertexArray_(vao_);
  glDrawArrays_(primitive_mode_, 0, vertex_size_);
}

void Lines::OnDestory(Context *context) {
  Clear();
}

void Lines::Clear() {
  glDeleteVertexArrays_(1, &vao_);
  glDeleteBuffers_(1, &vbo_);
}

Coord::Coord() {
  glm::vec3 translation = transform().translation();
  std::vector<glm::vec3> positions{glm::vec3(0, 0, 0), glm::vec3(1, 0, 0),
                                   glm::vec3(0, 0, 0), glm::vec3(0, 1, 0),
                                   glm::vec3(0, 0, 0), glm::vec3(0, 0, 1)};
  std::vector<glm::vec3> colors{glm::vec3(1, 0, 0), glm::vec3(1, 0, 0),
                                glm::vec3(0, 1, 0), glm::vec3(0, 1, 0),
                                glm::vec3(0, 0, 1), glm::vec3(0, 0, 1)};
  SetMesh({positions, colors, GL_LINES});
}