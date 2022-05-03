#pragma once

#include "engine/mesh.h"

class PlaneMesh : public engine::Mesh {
 public:
  PlaneMesh(const std::string& name = "plane_mesh");
};