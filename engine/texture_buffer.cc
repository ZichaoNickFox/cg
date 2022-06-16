#include "engine/texture_buffer.h"

namespace engine {

TextureBuffer::TextureBuffer() {
  glGenBuffers_(1, &buffer_);
}

TextureBuffer::~TextureBuffer() {
  glDeleteBuffers_(1, &buffer_);
}

}