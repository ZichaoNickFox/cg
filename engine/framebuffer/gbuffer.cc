#include "engine/framebuffer/gbuffer.h"

#include "glog/logging.h"
#include <memory>

#include "engine/debug.h"
#include "engine/gl.h"

namespace engine {
void GBuffer::Init(const Option& option) {
  size_ = option.size;
  option_ = option;

  glBindFramebuffer_(GL_FRAMEBUFFER, fbo_);

  for (int i = 0; i < kGBufferMRTLayout.size(); ++i) {
    const FramebufferAttachment& attachment = kGBufferMRTLayout[i];
    textures_.push_back(Texture());
    glGenTextures_(1, textures_[i].mutable_id());
    
    glBindTexture_(GL_TEXTURE_2D, textures_[i].id());
    glTexImage2D_(GL_TEXTURE_2D, 0, attachment.internal_type, option.size.x, option.size.y, 0,
                  attachment.format, attachment.type, NULL);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, attachment.texture_param_min_filter);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, attachment.texture_param_mag_filter);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, attachment.texture_param_wrap_s);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, attachment.texture_param_wrap_t);

    glFramebufferTexture2D_(GL_FRAMEBUFFER, attachment.attachment_unit, GL_TEXTURE_2D, textures_[i].id(), 0);
  }

  GLenum framebuffer_status = glCheckFramebufferStatus_(GL_FRAMEBUFFER);
  if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE) {
    CGCHECK(false) << "Frame Buffer Status Error : State - " << framebuffer_status;
  }

  glBindFramebuffer_(GL_FRAMEBUFFER, 0); 
}

void GBuffer::OnBind() {
  std::vector<GLuint> draw_attachment_units;
  for (int i = 0; i < kGBufferMRTLayout.size(); ++i) {
    if (kGBufferMRTLayout[i].is_draw_attachment_unit) {
      draw_attachment_units.push_back(kGBufferMRTLayout[i].attachment_unit);
    }
  }
  glDrawBuffers_(kGBufferMRTLayout.size(), draw_attachment_units.data());
}

void GBuffer::Clear() {
  glClear_(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void GBuffer::OnUnbind() {

}

Texture GBuffer::GetTexture(const std::string& layout) {
  for (int i = 0; i < kGBufferMRTLayout.size(); ++i) {
    if (kGBufferMRTLayout[i].name == layout) {
      return textures_[i];
    }
  }
  CGCHECK(false) << "Cannot find layout in GBUffer : " << layout;
  return Texture();
}

void GBuffer::BlitDepth(DepthFramebuffer* depth_framebuffer) {
  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_);
  if (depth_framebuffer == nullptr) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  } else {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, depth_framebuffer->fbo());
  }
  glBlitFramebuffer(0, 0, option_.size.x, option_.size.y,
                    0, 0, option_.size.x, option_.size.y,
                    GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}
}