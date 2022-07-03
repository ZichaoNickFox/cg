#pragma once

#include "renderer/framebuffer_attachment.h"

namespace renderer {
GLuint FramebufferAttachment::GetAttachmentBase() const {
  if (type == Type::kColor) {
    return GL_COLOR_ATTACHMENT0;
  } else if (type == Type::kDepth) {
    return GL_DEPTH_ATTACHMENT;
  } else if (type == Type::kStencil) {
    return GL_STENCIL_ATTACHMENT;
  } else {
    CGCHECK(false) << "Unsupported attachment type : " << type;
    return GL_NONE;
  }
}
} // namespace renderer
