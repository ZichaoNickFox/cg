#include "renderer/primitive.h"

#include "renderer/util.h"

namespace renderer {

SSBO PrimitiveRepo::BindSSBO(int binding_point) {
  SSBO res;
  res.Init(binding_point, util::VectorSizeInByte(primitives_), primitives_.data());
  return res;
}

const AABB& PrimitiveRepo::GetAABB(int index) const {
  CGCHECK(index >= 0 && index < primitives_.size());
  return primitives_.at(index).aabb; 
}

const Triangle& PrimitiveRepo::GetTriangle(int index) const {
  CGCHECK(index >= 0 && index < primitives_.size());
  return primitives_.at(index).triangle; 
}

void PrimitiveRepo::PushTriangle(const Triangle& triangle) {
  primitives_.push_back({triangle.AsAABB(), triangle, triangle.normal()});
}

} // namespace renderer