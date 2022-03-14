#pragma once

#include <glm/glm.hpp>

namespace engine {
class Transform {
 public:
  Transform() {}
  Transform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale);
  void SetTranslation(const glm::vec3 translation) { translation_ = translation; }
  void SetRotation(const glm::vec3 rotation) { rotation_ = rotation; }
  void SetScale(const glm::vec3 scale) { scale_ = scale; }
  glm::mat4 GetModelMatrix() const ;

  const glm::vec3& translation() const { return translation_; }
  // TODO : Rotate
  const glm::vec3& scale() const { return scale_; }

 private:
  glm::vec3 translation_ = glm::vec3(0, 0, 0);
  glm::vec3 rotation_ = glm::vec3(0, 0, 0);
  glm::vec3 scale_ = glm::vec3(1, 1, 1);
};
}