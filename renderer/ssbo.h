#pragma once

#include "base/debug.h"
#include "renderer/definition.h"
#include "rhi/device.h"

#include <glm/glm.hpp>
#include <memory>

namespace cg {

class SSBO {
 public:
  SSBO(int binding_point)
      : buffer_(rhi::GetDevice().CreateBuffer(rhi::BufferType::kStorage)),
        binding_point_(binding_point) {}

  template<typename DataStruct>
  void SetData(int size_in_byte, const DataStruct* data);

  template<typename DataType>
  DataType GetData();

  int binding_point() { return binding_point_; }
  rhi::BufferBindingDesc binding_desc() const {
    return {
        .buffer = buffer_.get(),
        .binding_point = static_cast<uint32_t>(binding_point_),
    };
  }

 private:
  std::unique_ptr<rhi::Buffer> buffer_;
  int binding_point_;
  bool inited_ = false;
};

template<typename DataStruct>
void SSBO::SetData(int size_in_byte, const DataStruct* data) {
  buffer_->SetData(size_in_byte, data, rhi::BufferUsage::kStreamCopy);
  if (!inited_) {
    rhi::GetDevice().ApplyBufferBindings({binding_desc()});
    inited_ = true;
  }
}

template<typename DataStruct>
DataStruct SSBO::GetData() {
  DataStruct data{};
  buffer_->ReadData({
      .offset_in_bytes = 0,
      .size_in_bytes = sizeof(DataStruct),
  }, &data);
  return data;
}

} // namespace cg
