#pragma once

#include "renderer/framebuffer_attachment.h"

namespace renderer {
GLuint FramebufferAttachment::GetAttachmentBase() const {
  if (type == AttachmentType::kColor) {
    return GL_COLOR_ATTACHMENT0;
  } else if (type == AttachmentType::kDepth) {
    return GL_DEPTH_ATTACHMENT;
  } else if (type == AttachmentType::kStencil) {
    return GL_STENCIL_ATTACHMENT;
  } else {
    CGCHECK(false) << "Unsupported attachment type : " << type;
    return GL_NONE;
  }
}
} // namespace renderer
