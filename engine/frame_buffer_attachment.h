#pragma once

#include <string>
#include "GL/glew.h"

namespace engine {
struct FrameBufferAttachment {
  std::string name;
  GLuint format;
  GLuint type;
  GLuint internal_type;
  uint32_t size_in_float;
  uint32_t size_in_byte;
  GLuint attachment_unit;
  bool is_draw_attachment_unit;
  GLuint texture_param_min_filter;
  GLuint texture_param_mag_filter;
  GLuint texture_param_wrap_s;
  GLuint texture_param_wrap_t;
};
} // namespace engine