#include "engine/transform.h"

#include <glog/logging.h>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>

namespace engine {

Transform::Transform(const glm::vec3& translation, const glm::quat& rotation, const glm::vec3& scale) {
  translation_ = translation;
  rotation_ = rotation;
  scale_ = scale;
}

glm::mat4 Transform::GetModelMatrix() const {
  glm::mat4 model(1.0);
  model = glm::translate(model, translation_);
  model = model * glm::toMat4(rotation_);
  model = glm::scale(model, scale_);
  return model;
}

}