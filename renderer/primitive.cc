#include "renderer/primitive.h"

#include "renderer/util.h"

namespace renderer {

void PrimitiveRepo::UpdateSSBO() {
  bool dirty = primitives_ != dirty_primitives_;
  if (dirty) {
    std::vector<PrimitiveGPU> primitive_gpus;
    std::transform(primitives_.begin(), primitives_.end(), std::back_inserter(primitive_gpus),
                   [] (const Primitive& primitive) { return PrimitiveGPU(primitive); });
    ssbo_.SetData(util::VectorSizeInByte(primitive_gpus), primitive_gpus.data());
    dirty_primitives_ = primitives_;
  }
}

PrimitiveRepo::PrimitiveGPU::PrimitiveGPU(const Primitive& primitive) {
  aabb_gpu = AABBGPU(primitive.aabb);
  triangle_gpu = TriangleGPU(primitive.triangle);
  normal_objectindex = glm::vec4(primitive.normal, primitive.object_index);
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