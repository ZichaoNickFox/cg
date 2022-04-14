#pragma once

#include "engine/mesh.h"

class SphereMesh : public engine::Mesh {
 public:
  SphereMesh(int refine = 4, const std::string& name = "sphere_mesh");
};