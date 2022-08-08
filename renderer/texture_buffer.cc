#include "renderer/texture_buffer.h"

namespace cg {

TextureBuffer::TextureBuffer() {
  glGenBuffers_(1, &buffer_);
}

TextureBuffer::~TextureBuffer() {
  glDeleteBuffers_(1, &buffer_);
}

}