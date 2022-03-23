#include "engine/frame_buffer/frame_buffer_render.h"

#include <string>

namespace engine {

void FrameBufferRender::Init(int width, int height, std::shared_ptr<Shader> shader) {
  glGenVertexArrays(1, &screen_vao_);
  glGenBuffers(1, &screen_vbo_);
  glBindVertexArray(screen_vao_);
  glBindBuffer(GL_ARRAY_BUFFER, screen_vbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(screen_vertices_), &screen_vertices_, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  shader_ = shader;
}

void FrameBufferRender::Render(GLuint texture){
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  shader_->Use();

  glBindVertexArray(screen_vao_);
  glDisable(GL_DEPTH_TEST);
  glBindTexture(GL_TEXTURE_2D, texture);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindVertexArray(0);
}
}