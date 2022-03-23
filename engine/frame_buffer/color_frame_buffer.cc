#include "engine/frame_buffer/color_frame_buffer.h"

#include "GL/glew.h"
#include "glog/logging.h"
#include <memory>

namespace engine {
void ColorFrameBuffer::Init(int inWidth, int inHeight) {
  width_ = inWidth;
  height_ = inHeight;

  // frame buffer object
  glGenFramebuffers(1, &fbo_);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

  // texture attachment
  glGenTextures(1,&texture_);
  glBindTexture(GL_TEXTURE_2D ,texture_);
  // set buffer function
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, inWidth, inHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_, 0);

  // render buffer object attachment
  glGenRenderbuffers(1, &rbo_);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
  // set buffer function
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, inWidth, inHeight);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    CHECK(false) << "Frame Buffer Status Error";
  }

  glBindFramebuffer(GL_FRAMEBUFFER,0); 
}

void ColorFrameBuffer::Clear() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
}