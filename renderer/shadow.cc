#include "renderer/shadow.h"

#include "renderer/ssbo.h"
#include "base/util.h"

namespace cg {

void ShadowRepo::UpdateSSBO() {
  bool dirty = !(shadows_ == dirty_shadows_);
  if (dirty) {
    ssbo_.SetData(util::VectorSizeInByte(shadows_), shadows_.data());
    dirty_shadows_ = shadows_;
  }
}

} // namespace cg