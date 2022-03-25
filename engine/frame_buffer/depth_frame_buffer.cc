#include "engine/frame_buffer/depth_frame_buffer.h"

#include "GL/glew.h"
#include "glog/logging.h"

#include "engine/texture.h"

namespace engine {
void DepthFrameBuffer::Init(int width, int height) {
  width_ = width;
  height_ = height;

  glGenFramebuffers(1, &fbo_);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

  Texture texture = texture::CreateTexture2D(width, height, GL_DEPTH_COMPONENT);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture.id(), 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindTexture(GL_TEXTURE_2D, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    BTCHECK(false) << "Frame Buffer Status Error";
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DepthFrameBuffer::Clear() {
  glClear(GL_DEPTH_BUFFER_BIT);
}
}