#pragma once

#include "engine/geometry.h"

namespace engine {

struct Primitive {
  AABB aabb;
  Triangle triangle;
  int primitive_index;
  const AABB& GetAABB() const { return aabb; }
};

}