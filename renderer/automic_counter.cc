#include "renderer/automic_counter.h"

#include "base/debug.h"

namespace cg {

AutomicCounter::AutomicCounter()
    : buffer_(rhi::GetDevice().CreateBuffer(rhi::BufferType::kAtomicCounter)) {}

AutomicCounter::~AutomicCounter() = default;

void AutomicCounter::Init(int binding_point) {
  binding_point_ = binding_point;
  buffer_->SetData(sizeof(uint32_t), nullptr, rhi::BufferUsage::kDynamic);
  rhi::GetDevice().ApplyBufferBindings({binding_desc()});
  inited_ = true;
}

void AutomicCounter::Reset(uint32_t value) {
  CGCHECK(inited_);
  buffer_->UpdateData(0, sizeof(value), &value);
}

}  // namespace cg
