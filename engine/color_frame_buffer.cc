#include "engine/color_frame_buffer.h"

#include "GL/glew.h"
#include "glog/logging.h"
#include <memory>

#include "engine/debug.h"

namespace engine {
void ColorFrameBuffer::Init(const Option& option) {
  width_ = option.width;
  height_ = option.height;
  option_ = option;

  // Frame buffer object
  glGenFramebuffers(1, &fbo_);
  SetFboNamePair(fbo_, option.name);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

  for (int i = 0; i < option.mrt; ++i) {
    textures_.push_back(Texture());
    glGenTextures(1, textures_[i].mutable_id());
    
    glBindTexture(GL_TEXTURE_2D, textures_[i].id());
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, option.width, option.height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures_[i].id(), 0);
  }

  // Depth frame buffer
  // Attention :ms_color_buffer must work with ms_depth_buffer
  textures_.push_back(Texture());
  glGenTextures(1, textures_[option.mrt].mutable_id());
  glBindTexture(GL_TEXTURE_2D, textures_[option.mrt].id());
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, option.width, option.height, 0, GL_DEPTH_COMPONENT,
      GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textures_[option.mrt].id(), 0);

  GLenum frame_buffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (frame_buffer_status != GL_FRAMEBUFFER_COMPLETE) {
    CGCHECK(false) << "Frame Buffer Status Error : State - " << frame_buffer_status;
  }

  glBindFramebuffer(GL_FRAMEBUFFER,0); 
}

void ColorFrameBuffer::OnBind() {
  std::vector<GLuint> attachments;
  for (int i = 0; i < option_.mrt; ++i) {
    attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
  }
  glDrawBuffers(option_.mrt, attachments.data());
}

void ColorFrameBuffer::Clear() {
  glClear(GL_DEPTH_BUFFER_BIT);
  CGCHECK(option_.clear_colors.size() == option_.mrt) << "Option's clear_color size need EQ to mrt";
  for (int i = 0; i < option_.mrt; ++i) {
    float clear_colors[] = {option_.clear_colors[i].x, option_.clear_colors[i].y, option_.clear_colors[i].z,
        option_.clear_colors[i].w};
    glClearBufferfv(GL_COLOR, i, clear_colors);
  }
}

void ColorFrameBuffer::OnUnbind() {

}

Texture ColorFrameBuffer::GetTexture(int i) {
  return textures_[i];
}
}