#include "playground/object/plane.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

Plane::Plane()
{
  float vertices[] = {
    // position          texcoord   normal
    -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    1.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
    -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f
  };

  glGenBuffers(1, &vbo_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  const int kPosLayout = 0;
  const int kTexcoordLayout = 1;
  const int kNormalLayout = 2;

  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);
  glEnableVertexAttribArray(kPosLayout);
  glEnableVertexAttribArray(kTexcoordLayout);
  glEnableVertexAttribArray(kNormalLayout);

  // Stride param must assign to vertex size 8 * sizeof(float)
  // Offset param means offset in a vertex
  glVertexAttribPointer(kPosLayout, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glVertexAttribPointer(kTexcoordLayout, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
  glVertexAttribPointer(kNormalLayout, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(5 * sizeof(float)));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void Plane::OnUpdate(Context *context) {

}

void Plane::OnRender(Context *context)
{
  glBindVertexArray(vao_);

  const engine::Camera& camera = context->camera();
  glm::mat4 project = camera.GetProjectMatrix();
  glm::mat4 view = camera.GetViewMatrix();
  glm::mat4 model = transform_.GetModelMatrix();
  material_.SetMat4("project", project);
  material_.SetMat4("view", view);
  material_.SetMat4("model", model);
  material_.PrepareShader();

  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Plane::OnDestory(Context *context) {
  glDeleteVertexArrays(1, &vao_);
  glDeleteBuffers(1, &vbo_);
}