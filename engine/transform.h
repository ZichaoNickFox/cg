#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

// Quaternion
// https://krasjet.github.io/quaternion/quaternion.pdf
namespace engine {
class Transform {
 public:
  Transform() {}
  Transform(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale);
  void SetTranslation(const glm::vec3 translation) { translation_ = translation; }
  void SetRotation(const glm::quat rotation) { rotation_ = glm::normalize(rotation); }
  void SetScale(const glm::vec3 scale) { scale_ = scale; }
  glm::mat4 GetModelMatrix() const;

  const glm::vec3& translation() const { return translation_; }
  const glm::quat& rotation() const { return rotation_; }
  const glm::vec3& scale() const { return scale_; }

  void Translate(const glm::vec3& translate) { translation_ += translate; }
  void Rotate(const glm::quat& quat) { rotation_ = glm::normalize(quat * rotation_); }
  void scaling(const glm::vec3& scaling) { scale_ *= scaling; };

  glm::vec3* mutable_translation() { return &translation_; }
  glm::vec3* mutable_scale() { return &scale_; }

 private:
  glm::vec3 translation_ = glm::vec3(0, 0, 0);
  glm::quat rotation_ = glm::quat(glm::vec3(0, 0, 0));
  glm::vec3 scale_ = glm::vec3(1, 1, 1);
};
}