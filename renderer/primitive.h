#pragma once

#include "renderer/definition.h"
#include "renderer/geometry.h"
#include "renderer/ssbo.h"

namespace renderer {

struct Primitive {
  AABB aabb;
  Triangle triangle;
  glm::vec3 normal;
  int object_index;

  const AABB& GetAABB() const { return aabb; }

  bool operator==(const Primitive& other) const {
    return memcmp(this, &other, sizeof(Primitive));
  }
};

class PrimitiveRepo {
 public:
  PrimitiveRepo() : ssbo_(SSBO_PRIMITIVE_REPO) {}
  void PushTriangle(const Triangle& triangle);

  int size() const { return primitives_.size(); }
  const std::vector<Primitive>& data() const { return primitives_; }

  const AABB& GetAABB(int index) const;
  const Triangle& GetTriangle(int index) const;

  void UpdateSSBO();

 private:
  std::vector<Primitive> primitives_;
  std::vector<Primitive> dirty_primitives_;
  SSBO ssbo_;
};

} // namespace renderer