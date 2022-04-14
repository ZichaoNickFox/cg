#include "playground/object/lines.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

#include "playground/util.h"

void Lines::SetData(const Data& data) {
  Clear();
  OnInit(data);
}

void Lines::OnInit(const Data& data) {
  primitive_mode_ = data.primitive_mode;
  vertex_size_ = data.points.size();
  line_width_ = data.line_width;

  CGCHECK(data.points.size() == data.colors.size());
  std::vector<glm::vec3> buffer(data.points.size() + data.colors.size(), glm::vec3());
  glGenBuffers(1, &vbo_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, util::VectorByteSize(buffer), buffer.data(), GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, util::VectorByteSize(data.points), data.points.data());
  glBufferSubData(GL_ARRAY_BUFFER, util::VectorByteSize(data.points), util::VectorByteSize(data.colors), data.colors.data());

  const int kPosLayout = 0;
  const int kColorLayout = 1;
  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);
  glEnableVertexAttribArray(kPosLayout);
  glEnableVertexAttribArray(kColorLayout);

  // Stride param must assign to vertex size 8 * sizeof(float)
  // Offset param means offset in a vertex
  glVertexAttribPointer(kPosLayout, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glVertexAttribPointer(kColorLayout, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void *>(util::VectorByteSize(data.points)));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void Lines::OnUpdate(Context *context) {

}

void Lines::OnRender(Context *context)
{ 
  glBindVertexArray(vao_);
  glDrawArrays(primitive_mode_, 0, vertex_size_);
}

void Lines::OnDestory(Context *context) {
  Clear();
}

void Lines::Clear() {
  glDeleteVertexArrays(1, &vao_);
  glDeleteBuffers(1, &vbo_);
}