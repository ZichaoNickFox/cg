#pragma once

#include "engine/geometry.h"
#include "engine/SSBO.h"

namespace engine {

struct Primitive {
  AABB aabb;
  Triangle triangle;
  const AABB& GetAABB() const { return aabb; }
};

class Primitives {
 public:
  void PushTriangle(const Triangle& triangle);

  int size() const { return primitives_.size(); }
  const std::vector<Primitive>& data() const { return primitives_; }

  const AABB& GetAABB(int index) const;
  const Triangle& GetTriangle(int index) const;

  SSBO AsSSBO(int binding_point);

 private:
  std::vector<Primitive> primitives_;
};

} // namespace engine