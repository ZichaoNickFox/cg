#pragma once

#include "renderer/definition.h"
#include "renderer/geometry.h"
#include "renderer/ssbo.h"

namespace renderer {

struct Primitive {
  Triangle triangle;
  AABB aabb;
  int material_index;

  const AABB& GetAABB() const { return aabb; }

  bool operator==(const Primitive& other) const = default;
};

class PrimitiveRepo {
 public:
  PrimitiveRepo() : ssbo_primitive_(SSBO_PRIMITIVE_REPO) {}
  void PushTriangle(const Triangle& triangle, int material_index);

  int num() const { return primitives_.size(); }
  const std::vector<Primitive>& data() const { return primitives_; }

  const Triangle& GetTriangle(int index) const;
  const AABB& GetAABB(int index) const;

  void UpdateSSBO(const std::vector<int>& primitive_sequence);

 private:
  struct PrimitiveGPU {
    PrimitiveGPU() = default;
    PrimitiveGPU(const Primitive& primitive);
    glm::vec4 a_materialIndex;
    glm::vec4 b;
    glm::vec4 c;
  };
  std::vector<Primitive> primitives_;
  std::vector<Primitive> dirty_primitives_;
  std::vector<int> dirty_primitive_sequence_;
  SSBO ssbo_primitive_;

  int acitive_material = -1;
};

} // namespace renderer