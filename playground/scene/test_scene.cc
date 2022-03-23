#include "playground/scene/test_scene.h"

#include "glog/logging.h"

#include "playground/scene/common.h"

void TestScene::OnEnter(Context* context) {
  float points[] = {
    // pos              // texcoord //color
    -1.0f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // left top
    0.5f,  -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, // right
    -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f  // left botton
  };
  const int kPosLayout = 0;
  const int kTexcoordLayout = 1;
  const int kColorLayout = 2;

  glGenBuffers(1, &vbo_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);
  glEnableVertexAttribArray(kPosLayout);
  glEnableVertexAttribArray(kTexcoordLayout);
  glEnableVertexAttribArray(kColorLayout);
  int pos_offset = 0;
  int texcoord_offset = 3 * sizeof(float);
  int color_offset = 5 * sizeof(float);
  int vertex_size = 8 * sizeof(float);
  glVertexAttribPointer(kPosLayout, 3, GL_FLOAT, GL_FALSE, vertex_size, reinterpret_cast<void*>(pos_offset));
  glVertexAttribPointer(kTexcoordLayout, 2, GL_FLOAT, GL_FALSE, vertex_size, reinterpret_cast<void*>(texcoord_offset));
  glVertexAttribPointer(kColorLayout, 3, GL_FLOAT, GL_FALSE, vertex_size, reinterpret_cast<void*>(color_offset));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  texture0_ = context->mutable_texture_repo()->GetOrLoadTexture("opengl0");
  texture1_ = context->mutable_texture_repo()->GetOrLoadTexture("opengl1");

  shader_ = context->mutable_shader_repo()->GetOrLoadShader("triangle");
  shader_.SetInt("texture0", texture0_.id());
  shader_.SetInt("texture1", texture1_.id());
}

void TestScene::OnUpdate(Context* context) {

}

void TestScene::OnGui(Context* context) {
  RenderGoToGallery(context); 
}

void TestScene::OnRender(Context* context) {
  // glActiveTexture(GL_TEXTURE0);
  // glBindTexture(GL_TEXTURE0, texture0_.id());
  // glActiveTexture(GL_TEXTURE1);
  // glBindTexture(GL_TEXTURE1, texture1_.id());

  shader_.Use();
  glBindVertexArray(vao_);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

void TestScene::OnExit(Context* context) {
  glDeleteBuffers(1, &vbo_);
  glDeleteVertexArrays(1, &vao_);
}