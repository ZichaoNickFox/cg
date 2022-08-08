#include "renderer/mesh/empty_mesh.h"

namespace cg {
EmptyMesh::EmptyMesh() {
  primitive_mode_ = GL_TRIANGLE_STRIP; 
  indices_.push_back(0);
  indices_.push_back(1);
  indices_.push_back(2);
  indices_.push_back(3);
  Setup();
}
}
