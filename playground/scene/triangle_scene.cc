#include "playground/scene/triangle_scene.h"

#include "glog/logging.h"

#include "playground/scene/common.h"

void TriangleScene::OnEnter(Context* context) {
  float points[] = {
    // pos              // texcoord //color
    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, // left top
    0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // right
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f  // left botton
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

  // Stride param must assign to vertex size 8 * sizeof(float)
  // Offset param means offset in a vertex
  glVertexAttribPointer(kPosLayout, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glVertexAttribPointer(kTexcoordLayout, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
  glVertexAttribPointer(kColorLayout, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  texture0_ = context->GetTexture("opengl0");
  texture1_ = context->GetTexture("opengl1");

  shader_ = context->GetShader("triangle");
  shader_.Use();
  // Set sampler to 0# texture unit
  shader_.SetInt("texture0", 0);
  // Set sampler to 1# texture unit
  shader_.SetInt("texture1", 1);

  // Activate texture unit GL_TEXTURE0. Then bind GL_TEXTURE_2D
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture0_.id());
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, texture1_.id());
}

void TriangleScene::OnUpdate(Context* context) {

}

void TriangleScene::OnGui(Context* context) {
  RenderFps(context); 
}

void TriangleScene::OnRender(Context* context) {
  shader_.Use();
  glBindVertexArray(vao_);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

void TriangleScene::OnExit(Context* context) {
  glDeleteBuffers(1, &vbo_);
  glDeleteVertexArrays(1, &vao_);
}