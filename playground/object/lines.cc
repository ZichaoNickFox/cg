#include "playground/object/lines.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

#include "playground/util.h"

void Lines::SetData(Context *context, const Data& data) {
  OnDestory(context);
  OnInit(context, data);
}

void Lines::OnInit(Context *context, const Data& data) {
  primitive_ = data.primitive;
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

  material_.PushShader(context->GetShader("lines"));
}

void Lines::OnUpdate(Context *context) {

}

void Lines::OnRender(Context *context)
{
  glBindVertexArray(vao_);

  const engine::Camera& camera = context->camera();
  glm::mat4 project = camera.GetProjectMatrix();
  glm::mat4 view = camera.GetViewMatrix();
  glm::mat4 model = glm::mat4(1);
  material_.SetMat4("project", project);
  material_.SetMat4("view", view);
  material_.SetMat4("model", model);
  material_.PrepareShader();

  glDrawArrays(primitive_, 0, vertex_size_);
}

void Lines::OnDestory(Context *context) {
  glDeleteVertexArrays(1, &vao_);
  glDeleteBuffers(1, &vbo_);
}