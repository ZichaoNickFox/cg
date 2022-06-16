#pragma once

#include "engine/gl.h"

namespace engine {

class TextureBuffer {
 public:
  TextureBuffer();
  ~TextureBuffer();

  template<typename DataType>
  void SetData(int size_in_byte, DataType* data);

 private:
  GLuint buffer_;
};

template<typename DataType>
void TextureBuffer::SetData(int size_in_byte, DataType* data) {
  glBindBuffer_(GL_TEXTURE_BUFFER, buffer_);
  glBufferData_(GL_TEXTURE_BUFFER, size_in_byte, data, GL_STATIC_DRAW);
  glBindBuffer_(GL_TEXTURE_BUFFER, 0);
}

}