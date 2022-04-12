#pragma once

#include "glm/glm.hpp"

namespace primitive_util {

void CalcTangentBitangent(const glm::vec3& position1, const glm::vec3& position2, const glm::vec3& position3,
                          const glm::vec2& texcoord1, const glm::vec2& texcoord2, const glm::vec2& texcoord3,
                          glm::vec3* triangle, glm::vec3* bitriangle);

}