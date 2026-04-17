#pragma once

#include <memory>
#include <cstdint>

#include "rhi/device.h"

namespace cg {

class TextureBuffer {
 public:
  TextureBuffer()
      : buffer_(rhi::GetDevice().CreateBuffer(rhi::BufferType::kTexture)) {}
  ~TextureBuffer() = default;

  template<typename DataType>
  void SetData(int size_in_byte, DataType* data);

 private:
  std::unique_ptr<rhi::Buffer> buffer_;
};

template<typename DataType>
void TextureBuffer::SetData(int size_in_byte, DataType* data) {
  buffer_->SetData(size_in_byte, data, rhi::BufferUsage::kStatic);
}

}
