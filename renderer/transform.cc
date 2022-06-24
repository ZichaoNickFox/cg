#include "renderer/transform.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>

#include "renderer/debug.h" 

namespace renderer {

Transform::Transform(const Transform& other)
    : translation_(other.translation_), rotation_(other.rotation_), scale_(other.scale_) {}

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

Transform operator*(const glm::mat4& transform, const Transform& source) {
  glm::vec3 translation = transform * glm::vec4(source.translation(), 1.0);
  glm::quat rotation = transform * glm::toMat4(source.rotation());
  glm::vec3 scale = transform * glm::vec4(source.scale(), 1.0);
  return Transform(translation, rotation, scale);
}
}