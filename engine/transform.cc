#include "engine/transform.h"

#include <glm/gtx/transform.hpp>

namespace engine {

Transform::Transform(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale) {
  translation_ = translation;
  rotation_ = rotation;
  scale_ = scale;
}

glm::mat4 Transform::GetModelMatrix() const {
  glm::mat4 model(1);
  model = glm::translate(model, translation_);
  model = glm::scale(model, scale_);
  // TODO : Rotate
  return model;
}

}