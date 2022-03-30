#include "engine/frame_buffer.h"

#include "GL/glew.h"
#include "glog/logging.h"
#include "assert.h"

#include "engine/debug.h"

namespace engine {

std::unordered_map<GLuint, std::string> FrameBuffer::fbo_names_ = {{0, "default"}};

void FrameBuffer::Bind() {
  CGCHECK(name_ != "") << "SetFboNamePair after gen fbo";
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &resumption_fbo_);
  glGetIntegerv(GL_VIEWPORT, resumption_viewport_);
  glViewport(0, 0, width_, height_);
  CGLOG(ERROR, false) << "Binding Framebuffer : " << fbo_names_[fbo_] << "   Previous : " << fbo_names_[resumption_fbo_];
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
  OnBind();
  Clear();
}

void FrameBuffer::Unbind() {
  CGLOG(ERROR, false) << "Unbinding Framebuffer : " << fbo_names_[fbo_] << "   Resuming : " << fbo_names_[resumption_fbo_];
  glBindFramebuffer(GL_FRAMEBUFFER, resumption_fbo_);
  glViewport(resumption_viewport_[0], resumption_viewport_[1], resumption_viewport_[2], resumption_viewport_[3]);
  OnUnbind();
}

void FrameBuffer::SetFboNamePair(GLuint fbo, const std::string& name) {
  CGCHECK(fbo == fbo_) << "This func need to be called after gen fbo";
  name_ = name;
  fbo_names_[fbo] = name;
}
}