#include "engine/primitive.h"

#include "engine/util.h"

namespace engine {

SSBO Primitives::AsSSBO(int binding_point) {
  SSBO res;
  res.Init(binding_point, util::VectorSizeInByte(primitives_), primitives_.data());
  return res;
}

const AABB& Primitives::GetAABB(int index) const {
  CGCHECK(index >= 0 && index < primitives_.size());
  return primitives_.at(index).aabb; 
}

const Triangle& Primitives::GetTriangle(int index) const {
  CGCHECK(index >= 0 && index < primitives_.size());
  return primitives_.at(index).triangle; 
}

void Primitives::PushTriangle(const Triangle& triangle) {
  primitives_.push_back({triangle.AsAABB(), triangle});
}

} // namespace engine