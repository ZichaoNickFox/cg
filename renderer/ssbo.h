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

 private:
  std::unique_ptr<rhi::Buffer> buffer_;
  int binding_point_;
  bool inited_ = false;
};

template<typename DataStruct>
void SSBO::SetData(int size_in_byte, const DataStruct* data) {
  buffer_->SetData(size_in_byte, data, rhi::BufferUsage::kStreamCopy);
  if (!inited_) {
    buffer_->BindBase(binding_point_);
    inited_ = true;
  }
}

template<typename DataStruct>
DataStruct SSBO::GetData() {
  void* ptr = CGCHECK_NOTNULL(buffer_->Map(rhi::MapAccess::kReadOnly));
  DataStruct data_ptr = *CGCHECK_NOTNULL(static_cast<DataStruct*>(ptr));
  buffer_->Unmap();
  return data_ptr;
}

} // namespace cg
