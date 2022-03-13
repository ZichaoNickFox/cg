#include "playground/scene/cube_world_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

#include "playground/scene/common.h"

void CubeWorldScene::OnEnter(Context *context)
{
  float vertices[] = {
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

    -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
    -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f
  };
  cube_positions_[0] = glm::vec3(0.0f, 0.0f, 0.0f);
  cube_positions_[1] = glm::vec3(2.0f, 5.0f, -15.0f);
  cube_positions_[2] = glm::vec3(-1.5f, -2.2f, -2.5f);
  cube_positions_[3] = glm::vec3(-3.8f, -2.0f, -12.3f);
  cube_positions_[4] = glm::vec3(2.4f, -0.4f, -3.5f);
  cube_positions_[5] = glm::vec3(-1.7f, 3.0f, -7.5f);
  cube_positions_[6] = glm::vec3(1.3f, -2.0f, -2.5f);
  cube_positions_[7] = glm::vec3(1.5f, 2.0f, -2.5f);
  cube_positions_[8] = glm::vec3(1.5f, 0.2f, -1.5f);
  cube_positions_[9] = glm::vec3(-1.3f, 1.0f, -1.5f);

  glGenBuffers(1, &vbo_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  const int kPosLayout = 0;
  const int kTexcoordLayout = 1;
  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);
  glEnableVertexAttribArray(kPosLayout);
  glEnableVertexAttribArray(kTexcoordLayout);

  // Stride param must assign to vertex size 8 * sizeof(float)
  // Offset param means offset in a vertex
  glVertexAttribPointer(kPosLayout, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glVertexAttribPointer(kTexcoordLayout, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  texture0_ = context->mutable_texture_repo()->GetTexture("cube_texture");

  shader_ = context->shader_repo().GetShader("cube");
  shader_.Use();
  // Set sampler to 0# texture unit
  shader_.SetInt("texture0", 0);

  // Activate texture unit GL_TEXTURE0. Then bind GL_TEXTURE_2D
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture0_.id());

  glEnable(GL_DEPTH_TEST);

  camera_.SetPosition(glm::vec3(0, 0, 0));
  camera_.SetFront(glm::vec3(0, 0, -1));
}

void CubeWorldScene::OnUpdate(Context *context)
{
  const float kMoveSpeed = 0.1;
  glm::vec3 pos = camera_.position();
  if (context->io().HadKeyInput("w")) {
    camera_.MoveForward(kMoveSpeed);
  } else if (context->io().HadKeyInput("s")) {
    camera_.MoveForward(-kMoveSpeed);
  } else if (context->io().HadKeyInput("a")) {
    camera_.MoveRight(-kMoveSpeed);
  } else if (context->io().HadKeyInput("d")) {
    camera_.MoveRight(kMoveSpeed);
  }

  if (context->io().right_button_pressed()) {
    const float kRotateSpeedFator = 0.001;
    double cursor_delta_x = context->io().GetCursorDeltaX() * kRotateSpeedFator;
    double cursor_delta_y = context->io().GetCursorDeltaY() * kRotateSpeedFator;
    camera_.RotateHorizontal(cursor_delta_x);
    camera_.RotateVerticle(cursor_delta_y);
  }
}

void CubeWorldScene::OnGui(Context *context)
{
  RenderGoToGallery(context);
}

void CubeWorldScene::OnRender(Context *context)
{
  shader_.Use();
  glBindVertexArray(vao_);

  glm::mat4 project = camera_.GetProjectMatrix();
  glm::mat4 view = camera_.GetViewMatrix();
  shader_.SetMat4("project", project);
  shader_.SetMat4("view", view);
  for (int i = 0; i < 10; ++i) {
    const glm::vec3& position = cube_positions_[i];
    // model
    glm::mat4 model(1);
    model = glm::translate(model, position);
    float angle = 20.f * i;
    model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
    shader_.SetMat4("model", model);

    glDrawArrays(GL_TRIANGLES, 0, 36);
  }
}

void CubeWorldScene::OnExit(Context *context)
{
  glDeleteBuffers(1, &vbo_);
  glDeleteVertexArrays(1, &vao_);
}