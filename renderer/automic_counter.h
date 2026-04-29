#pragma once

#include <cstdint>
#include <memory>

#include <glm/glm.hpp>

#include "rhi/device.h"

namespace cg {

class AutomicCounter {
 public:
  AutomicCounter();
  ~AutomicCounter();

  void Init(int binding_point);
  int binding_point() { return binding_point_; }
  void Reset(uint32_t value);
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

};
