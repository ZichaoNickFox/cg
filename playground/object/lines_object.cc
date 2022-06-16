#include "playground/object/lines_object.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

#include "engine/util.h"

LinesObject::Mesh::Mesh(const std::vector<glm::vec4>& in_points, const std::vector<glm::vec4>& in_colors,
                        GLuint in_primitive_mode)
    : points(in_points), colors(in_colors), primitive_mode(in_primitive_mode) {}

LinesObject::Mesh::Mesh(const std::vector<engine::AABB>& aabbs, const glm::vec4& color) {
  points.resize(aabbs.size() * 24);
  colors = std::vector<glm::vec4>(points.size(), color);
  primitive_mode = GL_LINES;
  int i = 0;
  for (const engine::AABB& aabb : aabbs) {
    glm::vec4 a{aabb.minimum.x, aabb.minimum.y, aabb.minimum.z, 1.0};
    glm::vec4 b{aabb.minimum.x, aabb.maximum.y, aabb.minimum.z, 1.0};
    glm::vec4 c{aabb.minimum.x, aabb.maximum.y, aabb.maximum.z, 1.0};
    glm::vec4 d{aabb.minimum.x, aabb.minimum.y, aabb.maximum.z, 1.0};
    glm::vec4 e{aabb.maximum.x, aabb.minimum.y, aabb.minimum.z, 1.0};
    glm::vec4 f{aabb.maximum.x, aabb.maximum.y, aabb.minimum.z, 1.0};
    glm::vec4 g{aabb.maximum.x, aabb.maximum.y, aabb.maximum.z, 1.0};
    glm::vec4 h{aabb.maximum.x, aabb.minimum.y, aabb.maximum.z, 1.0};
    points[i++] = a;  points[i++] = b;
    points[i++] = b;  points[i++] = c;
    points[i++] = c;  points[i++] = d;
    points[i++] = d;  points[i++] = a;
    points[i++] = a;  points[i++] = e;
    points[i++] = b;  points[i++] = f;
    points[i++] = c;  points[i++] = g;
    points[i++] = d;  points[i++] = h;
    points[i++] = e;  points[i++] = f;
    points[i++] = f;  points[i++] = g;
    points[i++] = g;  points[i++] = h;
    points[i++] = h;  points[i++] = e;
  }
}

void LinesObject::SetMesh(const Mesh& data) {
  Clear();
  OnInit(data);
}

void LinesObject::OnInit(const Mesh& data) {
  primitive_mode_ = data.primitive_mode;
  vertex_size_ = data.points.size();

  CGCHECK(data.points.size() == data.colors.size()) << " " << data.points.size() << ":" << data.colors.size();
  std::vector<glm::vec4> buffer(data.points.size() + data.colors.size(), glm::vec4());
  glGenBuffers_(1, &vbo_);
  glBindBuffer_(GL_ARRAY_BUFFER, vbo_);
  glBufferData_(GL_ARRAY_BUFFER, util::VectorSizeInByte(buffer), buffer.data(), GL_STATIC_DRAW);
  glBufferSubData_(GL_ARRAY_BUFFER, 0, util::VectorSizeInByte(data.points), data.points.data());
  glBufferSubData_(GL_ARRAY_BUFFER, util::VectorSizeInByte(data.points), util::VectorSizeInByte(data.colors), data.colors.data());

  const int kPosLayout = 0;
  const int kColorLayout = 1;
  glGenVertexArrays_(1, &vao_);
  glBindVertexArray_(vao_);
  glEnableVertexAttribArray_(kPosLayout);
  glEnableVertexAttribArray_(kColorLayout);

  // Stride param must assign to vertex size 8 * sizeof(float)
  // Offset param means offset in a vertex
  glVertexAttribPointer_(kPosLayout, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glVertexAttribPointer_(kColorLayout, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                         reinterpret_cast<void *>(util::VectorSizeInByte(data.points)));

  glBindBuffer_(GL_ARRAY_BUFFER, 0);
  glBindVertexArray_(0);
}

void LinesObject::OnUpdate(Context *context) {

}

void LinesObject::OnRender(Context *context, int instance_num) {
  material_.PrepareShader();
  glBindVertexArray_(vao_);
  glDrawArrays_(primitive_mode_, 0, vertex_size_);
}

void LinesObject::OnDestory(Context *context) {
  Clear();
}

void LinesObject::Clear() {
  glDeleteVertexArrays_(1, &vao_);
  glDeleteBuffers_(1, &vbo_);
}

Coord::Coord() {
  std::vector<glm::vec4> positions{glm::vec4(0, 0, 0, 1), glm::vec4(1, 0, 0, 1),
                                   glm::vec4(0, 0, 0, 1), glm::vec4(0, 1, 0, 1),
                                   glm::vec4(0, 0, 0, 1), glm::vec4(0, 0, 1, 1)};
  std::vector<glm::vec4> colors{glm::vec4(1, 0, 0, 1), glm::vec4(1, 0, 0, 1),
                                glm::vec4(0, 1, 0, 1), glm::vec4(0, 1, 0, 1),
                                glm::vec4(0, 0, 1, 1), glm::vec4(0, 0, 1, 1)};
  SetMesh({positions, colors, GL_LINES});
}