#include "engine/frame_buffer/frame_buffer.h"

#include "GL/glew.h"
#include "assert.h"

namespace engine {
std::shared_ptr<GLubyte> FrameBuffer::GetTextureData() {
  std::shared_ptr<GLubyte> pixels(new GLubyte[width_ * height_ * 3]);
  glReadPixels(0, 0, width_, height_, GL_RGB, GL_UNSIGNED_BYTE, reinterpret_cast<void*>(*pixels));
  return pixels;
}

void FrameBuffer::Bind() {
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &resumption_fbo_);
  glGetIntegerv(GL_VIEWPORT, resumption_viewport_);
  glViewport(0, 0, width_, height_);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
  glClear(GL_DEPTH_BUFFER_BIT);
}

void FrameBuffer::Unbind() {
  glBindFramebuffer(GL_FRAMEBUFFER, resumption_fbo_);
  glViewport(resumption_viewport_[0], resumption_viewport_[1], resumption_viewport_[2], resumption_viewport_[3]);
}
}