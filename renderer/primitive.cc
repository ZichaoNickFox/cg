#include "renderer/primitive.h"

#include <glm/gtx/string_cast.hpp>

#include "base/util.h"

namespace cg {

void PrimitiveRepo::UpdateSSBO(const std::vector<int>& primitive_sequence) {
  CGCHECK(primitive_sequence.size() == primitives_.size());
  bool dirty = primitives_ != dirty_primitives_ || dirty_primitive_sequence_ != primitive_sequence;
  if (dirty) {
    std::vector<PrimitiveGPU> primitive_gpus(primitive_sequence.size());
    for (int i = 0; i < primitives_.size(); ++i) {
      int sequence = primitive_sequence[i];
      primitive_gpus[i] = PrimitiveGPU(primitives_[sequence]);
    }
    ssbo_primitive_.SetData(util::VectorSizeInByte(primitive_gpus), primitive_gpus.data());

    dirty_primitives_ = primitives_;
    dirty_primitive_sequence_ = primitive_sequence;
  }
}

PrimitiveRepo::PrimitiveGPU::PrimitiveGPU(const Primitive& primitive) {
  a_materialIndex = glm::vec4(primitive.triangle.a, primitive.material_index);
  b = glm::vec4(primitive.triangle.b, 0.0);
  c = glm::vec4(primitive.triangle.c, 0.0);
}

const AABB& PrimitiveRepo::GetAABB(int index) const {
  CGCHECK(index >= 0 && index < primitives_.size());
  return primitives_.at(index).aabb; 
}

const Triangle& PrimitiveRepo::GetTriangle(int index) const {
  CGCHECK(index >= 0 && index < primitives_.size());
  return primitives_.at(index).triangle; 
}

void PrimitiveRepo::PushTriangle(const Triangle& triangle, int material_index) {
  primitives_.push_back({triangle, triangle.AsAABB(), material_index});
}

} // namespace cg