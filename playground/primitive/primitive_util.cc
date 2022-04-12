#include "playground/primitive/primitive_util.h"

#include "glog/logging.h"

namespace primitive_util {

void CalcTangentBitangent(const glm::vec3& pos1, const glm::vec3& pos2, const glm::vec3& pos3,
                          const glm::vec2& texcoord1, const glm::vec2& texcoord2, const glm::vec2& texcoord3,
                          glm::vec3* tangent, glm::vec3* bitangent) {
  glm::vec3 edge1 = pos2 - pos1;
  glm::vec3 edge2 = pos3 - pos1;
  glm::vec2 deltaUV1 = texcoord2 - texcoord1;
  glm::vec2 deltaUV2 = texcoord3 - texcoord1;
  float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
  tangent->x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
  tangent->y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
  tangent->z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
  *tangent = glm::normalize(*tangent);

  bitangent->x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
  bitangent->y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
  bitangent->z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
  *bitangent = glm::normalize(*bitangent);
}

}