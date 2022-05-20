#include "engine/framebuffer.h"

#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glog/logging.h"
#include "assert.h"

#include "engine/debug.h"
#include "engine/gl.h"

namespace engine {
Framebuffer::Framebuffer() {
  glGenFramebuffers_(1, &fbo_);
}

// TODO: Press x to close would cause here crash, but ESC not.
Framebuffer::~Framebuffer() {
  glDeleteFramebuffers_(1, &fbo_);
}

void Framebuffer::Init(const Option& option) {
  option_ = option;

  glBindFramebuffer_(GL_FRAMEBUFFER, fbo_);
  std::vector<uint32_t> texture_unit_indices(FramebufferAttachment::AttachmentTypeNum, 0);
  for (int i = 0; i < option_.attachments.size(); ++i) {
    const FramebufferAttachment& attachment = option_.attachments[i];
    CGCHECK(textures_.count(attachment.name) == 0) << "Attachments has same name : " << attachment.name;
    Texture* texture = &textures_[attachment.name];
    glGenTextures_(1, texture->mutable_id());
    glBindTexture_(GL_TEXTURE_2D, texture->id());
    glTexImage2D_(GL_TEXTURE_2D, 0, attachment.texture_internal_type, option.size.x, option.size.y, 0,
      attachment.texture_format, attachment.texture_type, NULL);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, attachment.texture_param_min_filter);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, attachment.texture_param_mag_filter);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, attachment.texture_param_wrap_s);
    glTexParameteri_(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, attachment.texture_param_wrap_t);

    GLuint gl_attachment = attachment.GetAttachmentBase() + texture_unit_indices[attachment.type]++;
    glFramebufferTexture2D_(GL_FRAMEBUFFER, gl_attachment, GL_TEXTURE_2D, texture->id(), 0);
    if (attachment.type == FramebufferAttachment::kColor) {
      draw_buffers_.push_back(gl_attachment);
      draw_buffers_clear_color_.push_back(attachment.clear_color);
    }
  }
  GLenum framebuffer_status = glCheckFramebufferStatus_(GL_FRAMEBUFFER);
  if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE) {
    CGCHECK(false) << "Framebuffer Status Error : State - " << framebuffer_status;
  }
  glBindFramebuffer_(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Bind() {
  glGetIntegerv_(GL_VIEWPORT, resumption_viewport_);
  glGetIntegerv_(GL_FRAMEBUFFER_BINDING, &resumption_fbo_);
  glViewport_(0, 0, option_.size.x, option_.size.y);
  glBindFramebuffer_(GL_FRAMEBUFFER, fbo_);

  glClear_(GL_DEPTH_BUFFER_BIT);
  if (draw_buffers_.size() > 0) {
    glDrawBuffers_(draw_buffers_.size(), draw_buffers_.data());
    for (int i = 0; i < draw_buffers_.size(); ++i) {
      glClearBufferfv_(GL_COLOR, i, glm::value_ptr(draw_buffers_clear_color_[i]));
    }
  } else {
    glDrawBuffer_(GL_NONE);
  }
}

void Framebuffer::Unbind() {
  glBindFramebuffer_(GL_FRAMEBUFFER, resumption_fbo_);
  glViewport_(resumption_viewport_[0], resumption_viewport_[1], resumption_viewport_[2], resumption_viewport_[3]);
}

Texture Framebuffer::GetTexture(const std::string& name) {
  CGCHECK(textures_.count(name) == 1) << " Cannot find texture in framebuffer " << name;
  return textures_[name];
}
}