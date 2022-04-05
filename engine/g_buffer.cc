#include "engine/g_buffer.h"

#include "GL/glew.h"
#include "glog/logging.h"
#include <memory>

#include "engine/debug.h"
#include "engine/pass.h"

namespace engine {
void GBuffer::Init(const Option& option) {
  width_ = option.width;
  height_ = option.height;
  option_ = option;

  // Frame buffer object
  glGenFramebuffers(1, &fbo_);
  SetFboNamePair(fbo_, option.name);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

  for (int i = 0; i < kGBufferMRTLayout.size(); ++i) {
    const FrameBufferAttachment& attachment = kGBufferMRTLayout[i];
    textures_.push_back(Texture());
    glGenTextures(1, textures_[i].mutable_id());
    
    glBindTexture(GL_TEXTURE_2D, textures_[i].id());
    glTexImage2D(GL_TEXTURE_2D, 0, attachment.internal_type, option.width, option.height, 0,
        attachment.format, attachment.type, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, attachment.texture_param_min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, attachment.texture_param_mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, attachment.texture_param_wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, attachment.texture_param_wrap_t);

    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment.attachment_unit, GL_TEXTURE_2D, textures_[i].id(), 0);
  }

  GLenum frame_buffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (frame_buffer_status != GL_FRAMEBUFFER_COMPLETE) {
    CGCHECK(false) << "Frame Buffer Status Error : State - " << frame_buffer_status;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0); 
}

void GBuffer::OnBind() {
  std::vector<GLuint> draw_attachment_units;
  for (int i = 0; i < kGBufferMRTLayout.size(); ++i) {
    if (kGBufferMRTLayout[i].is_draw_attachment_unit) {
      draw_attachment_units.push_back(kGBufferMRTLayout[i].attachment_unit);
    }
  }
  glDrawBuffers(kGBufferMRTLayout.size(), draw_attachment_units.data());
}

void GBuffer::Clear() {
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void GBuffer::OnUnbind() {

}

Texture GBuffer::GetTexture(int i) {
  return textures_[i];
}
}