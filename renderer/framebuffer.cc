#include "renderer/framebuffer.h"

#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glog/logging.h"
#include "assert.h"

#include "renderer/debug.h"
#include "renderer/gl.h"

namespace renderer {
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

    Texture attachment_texture = CreateAttachmentTexture(attachment);

    textures_[attachment.name] = attachment_texture;
    GLuint attachment_unit = attachment.GetAttachmentBase() + texture_unit_indices[attachment.type]++;
    glFramebufferTexture2D_(GL_FRAMEBUFFER, attachment_unit, GL_TEXTURE_2D, attachment_texture.id(), 0);
    if (attachment.type == FramebufferAttachment::kColor) {
      draw_buffers_.push_back(attachment_unit);
    }
  }
  GLenum framebuffer_status = glCheckFramebufferStatus_(GL_FRAMEBUFFER);
  if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE) {
    CGCHECK(false) << "Framebuffer Status Error : State - " << framebuffer_status;
  }
  glBindFramebuffer_(GL_FRAMEBUFFER, 0);

  inited_ = true;
}

void Framebuffer::Bind() {
  CGCHECK(inited_);
  glGetIntegerv_(GL_VIEWPORT, resumption_viewport_);
  glGetIntegerv_(GL_FRAMEBUFFER_BINDING, &resumption_fbo_);
  glViewport_(0, 0, option_.size.x, option_.size.y);
  glBindFramebuffer_(GL_FRAMEBUFFER, fbo_);
  glClearColor_(option_.clear_color.r, option_.clear_color.g, option_.clear_color.b, option_.clear_color.a);
  glClear_(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  for (int i = 0; i < option_.attachments.size(); ++i) {
    const FramebufferAttachment& attachment = option_.attachments[i];
    if (attachment.type == FramebufferAttachment::kColor && attachment.clear_type == FramebufferAttachment::kClear) {
      glClearBufferfv_(GL_COLOR, i, glm::value_ptr(option_.clear_color));
    }
  }
  glDrawBuffers_(draw_buffers_.size(), draw_buffers_.data());
}

void Framebuffer::Unbind() {
  glBindFramebuffer_(GL_FRAMEBUFFER, resumption_fbo_);
  glViewport_(resumption_viewport_[0], resumption_viewport_[1], resumption_viewport_[2], resumption_viewport_[3]);
}

Texture Framebuffer::GetTexture(const std::string& name) {
  CGCHECK(textures_.count(name) == 1) << " Cannot find texture in framebuffer : " << name;
  return textures_[name];
}

void Framebuffer::Blit(Framebuffer* framebuffer) {
  glBindFramebuffer_(GL_READ_FRAMEBUFFER, fbo_);
  glBindFramebuffer_(GL_DRAW_FRAMEBUFFER, framebuffer ? framebuffer->fbo() : 0);
  glBlitFramebuffer_(0, 0, option_.size.x, option_.size.y, 0, 0, option_.size.x, option_.size.y,
                     GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

Texture Framebuffer::CreateAttachmentTexture(const FramebufferAttachment& attachment) {
  Texture::Meta texture_meta = attachment.texture_meta;
  texture_meta.width = option_.size.x;
  texture_meta.height = option_.size.y;
  if (attachment.texture_meta.gl_internal_format == GL_RGBA32F) {
    std::vector<glm::vec4> data(texture_meta.data_size_in_byte() / sizeof(glm::vec4), kBlack);
    return CreateTexture2D(texture_meta, {(data.data())});
  } else if (attachment.texture_meta.gl_internal_format == GL_DEPTH_COMPONENT32F) {
    std::vector<float> data(texture_meta.data_size_in_byte() / sizeof(float), 0.0);
    return CreateTexture2D(texture_meta, {(data.data())});
  } else if (attachment.texture_meta.gl_internal_format == GL_RGB32F) {
    std::vector<glm::vec3> data(texture_meta.data_size_in_byte() / sizeof(glm::vec3), glm::vec3(0, 0, 0));
    return CreateTexture2D(texture_meta, {(data.data())});
  } else if (attachment.texture_meta.gl_internal_format == GL_RG32F) {
    std::vector<glm::vec2> data(texture_meta.data_size_in_byte() / sizeof(glm::vec2), glm::vec2(0, 0));
    return CreateTexture2D(texture_meta, {(data.data())});
  } else if (attachment.texture_meta.gl_internal_format == GL_R32UI) {
    std::vector<unsigned int> data(texture_meta.data_size_in_byte() / sizeof(unsigned int), 0);
    return CreateTexture2D(texture_meta, {(data.data())});
  } else {
    CGCHECK(false) << " Unsupported Internal format"
                   << std::hex << attachment.texture_meta.gl_internal_format << std::dec;
    return Texture();
  }
}
}