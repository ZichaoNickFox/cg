#include "engine/framebuffer/depth_framebuffer.h"

#include "glog/logging.h"

#include "engine/gl.h"
#include "engine/texture.h"

namespace engine {
void DepthFramebuffer::Init(const Option& option) {
  size_ = option.size;

  glBindFramebuffer_(GL_FRAMEBUFFER, fbo_);

  GLuint depth_texture;
  glGenTextures_(1, &depth_texture);
  textures_.push_back(Texture(depth_texture, engine::Texture::Texture2D));
  CHECK(textures_.size() == 1) << "Depth should only have 1 texture";
  glBindTexture_(GL_TEXTURE_2D, textures_[0].id());
  glTexImage2D_(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, option.size.x, option.size.y, 0, GL_DEPTH_COMPONENT,
      GL_FLOAT, NULL);
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textures_[0].id(), 0);
  glDrawBuffer_(GL_NONE);
  glReadBuffer_(GL_NONE);

  if (glCheckFramebufferStatus_(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    CGCHECK(false) << "Frame Buffer Status Error";
  }

  glBindFramebuffer_(GL_FRAMEBUFFER, 0);
}

void DepthFramebuffer::OnBind() {
}

void DepthFramebuffer::Clear() {
  glClear_(GL_DEPTH_BUFFER_BIT);
}

void DepthFramebuffer::OnUnbind() {
}

Texture DepthFramebuffer::GetDepthTexture(int i) {
  return textures_[i];
}
}