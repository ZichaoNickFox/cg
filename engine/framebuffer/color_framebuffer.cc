#include "engine/framebuffer/color_framebuffer.h"

#include "glog/logging.h"
#include <memory>

#include "engine/debug.h"

namespace engine {
void ColorFramebuffer::Init(const Option& option) {
  size_ = option.size;
  option_ = option;

  glBindFramebuffer_(GL_FRAMEBUFFER, fbo_);

  for (int i = 0; i < option.mrt; ++i) {
    textures_.push_back(Texture());
    glGenTextures_(1, textures_[i].mutable_id());
    
    glBindTexture_(GL_TEXTURE_2D, textures_[i].id());
    glTexImage2D_(GL_TEXTURE_2D, 0, option.internal_format, option.size.x, option.size.y,
                  0, option.format, option.type, NULL);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures_[i].id(), 0);
  }

  // Depth frame buffer
  // Attention :ms_color_buffer must work with ms_depth_buffer
  textures_.push_back(Texture());
  glGenTextures_(1, textures_[option.mrt].mutable_id());
  glBindTexture_(GL_TEXTURE_2D, textures_[option.mrt].id());
  glTexImage2D_(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, option.size.x, option.size.y, 0, GL_DEPTH_COMPONENT,
      GL_FLOAT, NULL);
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textures_[option.mrt].id(), 0);

  GLenum framebuffer_status = glCheckFramebufferStatus_(GL_FRAMEBUFFER);
  if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE) {
    CGCHECK(false) << "Frame Buffer Status Error : State - " << framebuffer_status;
  }

  glBindFramebuffer_(GL_FRAMEBUFFER, 0); 
}

void ColorFramebuffer::OnBind() {
  std::vector<GLuint> attachments;
  for (int i = 0; i < option_.mrt; ++i) {
    attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
  }
  glDrawBuffers_(option_.mrt, attachments.data());
}

void ColorFramebuffer::Clear() {
  glClear_(GL_DEPTH_BUFFER_BIT);
  for (int i = 0; i < option_.mrt; ++i) {
    glClearBufferfv_(GL_COLOR, i, &option_.clear_color.x);
  }
}

void ColorFramebuffer::OnUnbind() {
}

Texture ColorFramebuffer::GetColorTexture(int i) {
  return textures_[i];
}

Texture ColorFramebuffer::GetDepthTexture() {
  return textures_.back();
}

std::vector<GLubyte> ColorFramebuffer::GetColorTextureData(int i) {
  // TODO size
  glBindTexture_(GL_TEXTURE_2D, textures_[i].id());
  int size = option_.size.x * option_.size.y * 4 * 4;
  std::vector<GLubyte> res(size);
  glGetTexImage_(GL_TEXTURE_2D, 0, option_.format, option_.type, res.data());
  glBindTexture_(GL_TEXTURE_2D, 0);
  return res;
}
} // namespace engine