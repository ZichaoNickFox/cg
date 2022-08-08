#pragma once

#include "renderer/mesh.h"

class SphereMesh : public cg::Mesh {
 public:
  SphereMesh(int refine = 4);

 private:
  void FillPositionNormal(int refine, std::vector<glm::vec3>* triangles);
  void FillTexcoord(const std::vector<glm::vec3>& triangles, std::vector<glm::vec2>* texcoords);
  void FillTangentBitangent(const std::vector<glm::vec3>& triangles, const std::vector<glm::vec2>& texcoords,
                            std::vector<glm::vec3>* tangents, std::vector<glm::vec3>* bitangent);
};