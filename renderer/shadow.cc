#include "renderer/shadow.h"

#include "renderer/ssbo.h"
#include "renderer/util.h"

namespace renderer {

SSBO ShadowRepo::BindSSBO(int binding_point) {
  SSBO res;
  res.Init(binding_point, util::VectorSizeInByte(shadows_), shadows_.data());
  return res;
}

} // namespace renderer