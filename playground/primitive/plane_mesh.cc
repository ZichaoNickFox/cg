#include "playground/primitive/plane_mesh.h"

#include "playground/primitive/primitive_util.h"

PlaneMesh::PlaneMesh() {
  std::vector<glm::vec3> positions{
    glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(1.0f, 0.0f, -1.0f), glm::vec3(1.0f, 0.0f, 1.0f),
    glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, -1.0f),
  };
  SetPositions(positions);
  std::vector<glm::vec3> normals{
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
  };
  SetNormals(normals);
  std::vector<glm::vec2> texcoords{
    glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f),
    glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f),
  };
  SetTexcoords(texcoords);
  std::vector<glm::vec3> tangents;
  std::vector<glm::vec3> bitangents;
  for (int i = 0; i < positions.size(); i += 3) {
    glm::vec3 tangent;
    glm::vec3 bitangent;
    primitive_util::CalcTangentBitangent(positions[i], positions[i + 1], positions[i + 2], 
                                         texcoords[i], texcoords[i + 1], texcoords[i + 2],
                                         &tangent, &bitangent);
    tangents.push_back(tangent);
    tangents.push_back(tangent);
    tangents.push_back(tangent);
    bitangents.push_back(bitangent);
    bitangents.push_back(bitangent);
    bitangents.push_back(bitangent);
  }
  SetTangent(tangents);
  SetBitangent(bitangents);
}