#include "engine/framebuffer/ms_framebuffer.h"

#include "glog/logging.h"
#include <memory>

#include "engine/debug.h"
#include "engine/framebuffer/color_framebuffer.h"
#include "engine/gl.h"

namespace engine {
void MSFramebuffer::CheckSupportMSNum(GLuint fbo, int num) {
  int available_count;
  glGetInternalformativ_(GL_RENDERBUFFER, GL_RGBA8, GL_NUM_SAMPLE_COUNTS, 1, &available_count);
  std::vector<GLint> samples(available_count);
  glGetInternalformativ_(GL_RENDERBUFFER, GL_RGBA8, GL_SAMPLES, (GLsizei)available_count, samples.data());
  bool found = false;
  for (int i = 0; i < available_count; i++){
    if (samples[i] == num) {
      found = true;
    }
  }
  CGCHECK(found) << " Unsupported ms num " << num;
}

void MSFramebuffer::Init(const Option& option) {
CHECK(false);
/*
  option_ = option;

  glBindFramebuffer_(GL_FRAMEBUFFER, fbo_);
  CheckSupportMSNum(fbo_, option.ms_num);

  for (int i = 0; i < option.mrt; ++i) {
    textures_.push_back(Texture());
    glGenTextures_(1, textures_[i].mutable_id());
    CGCHECK(option.ms_num > 1) << " : If don't need MS, using color_framebuffer";
    glBindTexture_(GL_TEXTURE_2D_MULTISAMPLE, textures_[i].id());
    glTexImage2DMultisample_(GL_TEXTURE_2D_MULTISAMPLE, option.ms_num, GL_RGBA32F, option.size.x,
        option.size.y, true);
    glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D_MULTISAMPLE, textures_[i].id(), 0);
  }

  // Depth frame buffer
  // Attention :ms_color_buffer must work with ms_depth_buffer
  textures_.push_back(Texture());
  glGenTextures_(1, textures_[option.mrt].mutable_id());
  glBindTexture_(GL_TEXTURE_2D_MULTISAMPLE, textures_[option.mrt].id());
  glTexImage2DMultisample_(GL_TEXTURE_2D_MULTISAMPLE, option.ms_num, GL_DEPTH_COMPONENT32F, option.size.x,
      option.size.y, true);
  glFramebufferTexture2D_(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, textures_[option.mrt].id(), 0);

  GLenum framebuffer_status = glCheckFramebufferStatus_(GL_FRAMEBUFFER);
  if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE) {
    CGCHECK(false) << "Frame Buffer Status Error : State - " << framebuffer_status;
  }

  glBindFramebuffer_(GL_FRAMEBUFFER,0); 
  */
}

void MSFramebuffer::OnBind() {
  std::vector<GLuint> attachments;
  for (int i = 0; i < option_.mrt; ++i) {
    attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
  }
  glDrawBuffers_(option_.mrt, attachments.data());
}

void MSFramebuffer::Clear() {
  glClear_(GL_DEPTH_BUFFER_BIT);
  CGCHECK(option_.clear_colors.size() == option_.mrt) << "Option's clear_color size need EQ to mrt";
  for (int i = 0; i < option_.mrt; ++i) {
    float clear_colors[] = {option_.clear_colors[i].x, option_.clear_colors[i].y, option_.clear_colors[i].z,
        option_.clear_colors[i].w};
    glClearBufferfv_(GL_COLOR, i, clear_colors);
  }
}

void MSFramebuffer::OnUnbind() {

}

Texture MSFramebuffer::GetColorTexture(int i) {
  return textures_[i];
}

Texture MSFramebuffer::GetDepthTexture(int i) {
  return textures_.back();
}

void MSFramebuffer::Blit(ColorFramebuffer* color_framebuffer) {
  glBindFramebuffer_(GL_READ_FRAMEBUFFER, fbo_);
  glBindFramebuffer_(GL_DRAW_FRAMEBUFFER, color_framebuffer->fbo());
  glBlitFramebuffer_(0, 0, option_.size.x, option_.size.y,
                     0, 0, option_.size.x, option_.size.y,
                     GL_COLOR_BUFFER_BIT, GL_NEAREST);
}
}