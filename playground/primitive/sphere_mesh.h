#pragma once

#include "engine/mesh.h"

class SphereMesh : public engine::Mesh {
 public:
  SphereMesh(int refine = 4);
};