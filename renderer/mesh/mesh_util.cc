#include "renderer/mesh/mesh_util.h"

#include "glog/logging.h"

namespace mesh_util {

void CalcTangentBitangent(const glm::vec3& pos0, const glm::vec3& pos1, const glm::vec3& pos2,
                          const glm::vec2& texcoord0, const glm::vec2& texcoord1, const glm::vec2& texcoord2,
                          glm::vec3* tangent, glm::vec3* bitangent) {
  glm::vec3 edge1 = pos1 - pos0;
  glm::vec3 edge2 = pos2 - pos0;
  glm::vec2 deltaUV1 = texcoord1 - texcoord0;
  glm::vec2 deltaUV2 = texcoord2 - texcoord0;
  float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
  *tangent = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
  *tangent = glm::normalize(*tangent);

  *bitangent = f * (-deltaUV2.x * edge1 + deltaUV1.x * edge2);
  *bitangent = glm::normalize(*bitangent);
}

} // namespace mesh_util