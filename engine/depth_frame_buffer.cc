#include "engine/depth_frame_buffer.h"

#include "GL/glew.h"
#include "glog/logging.h"

#include "engine/texture.h"

namespace engine {
void DepthFrameBuffer::Init(const Option& option) {
  size_ = option.size;

  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

  textures_.push_back(Texture());
  CHECK(textures_.size() == 1) << "Depth should only have 1 texture";
  glGenTextures(1, textures_[0].mutable_id());
  glBindTexture(GL_TEXTURE_2D, textures_[0].id());
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, option.size.x, option.size.y, 0, GL_DEPTH_COMPONENT,
      GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textures_[0].id(), 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    CGCHECK(false) << "Frame Buffer Status Error";
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DepthFrameBuffer::OnBind() {
}

void DepthFrameBuffer::Clear() {
  glClear(GL_DEPTH_BUFFER_BIT);
}

void DepthFrameBuffer::OnUnbind() {
}

Texture DepthFrameBuffer::GetDepthTexture(int i) {
  return textures_[i];
}
}