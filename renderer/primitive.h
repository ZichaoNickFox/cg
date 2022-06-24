#pragma once

#include "renderer/geometry.h"
#include "renderer/ssbo.h"

namespace renderer {

struct Primitive {
  AABB aabb;
  Triangle triangle;
  glm::vec3 normal;
  int object_index;
  const AABB& GetAABB() const { return aabb; }
};

class PrimitiveRepo {
 public:
  void PushTriangle(const Triangle& triangle);

  int size() const { return primitives_.size(); }
  const std::vector<Primitive>& data() const { return primitives_; }

  const AABB& GetAABB(int index) const;
  const Triangle& GetTriangle(int index) const;

  SSBO BindSSBO(int binding_point);

 private:
  std::vector<Primitive> primitives_;
};

} // namespace renderer