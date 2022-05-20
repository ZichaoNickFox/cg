#pragma once

#include "engine/framebuffer_attachment.h"

namespace engine {
GLuint FramebufferAttachment::GetAttachmentBase() const {
  if (type == AttachmentType::kColor) {
    return GL_COLOR_ATTACHMENT0;
  } else if (type == AttachmentType::kDepth) {
    return GL_DEPTH_ATTACHMENT;
  } else if (type == AttachmentType::kStencil) {
    return GL_STENCIL_ATTACHMENT;
  } else {
    CGCHECK(false) << "Unsupported attachment type : " << type;
  }
}
} // namespace engine
