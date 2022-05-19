#include "engine/framebuffer/framebuffer.h"

#include "glog/logging.h"
#include "assert.h"

#include "engine/debug.h"
#include "engine/gl.h"

namespace engine {
Framebuffer::Framebuffer() {
  glGenFramebuffers_(1, &fbo_);
}

Framebuffer::~Framebuffer() {
  glDeleteFramebuffers_(1, &fbo_);
}

void Framebuffer::Bind() {
  glGetIntegerv_(GL_VIEWPORT, resumption_viewport_);
  glGetIntegerv_(GL_FRAMEBUFFER_BINDING, &resumption_fbo_);
  glViewport_(0, 0, size_.x, size_.y);
  glBindFramebuffer_(GL_FRAMEBUFFER, fbo_);
  OnBind();
  Clear();
}

void Framebuffer::Unbind() {
  glBindFramebuffer_(GL_FRAMEBUFFER, resumption_fbo_);
  glViewport_(resumption_viewport_[0], resumption_viewport_[1], resumption_viewport_[2], resumption_viewport_[3]);
  OnUnbind();
}
}