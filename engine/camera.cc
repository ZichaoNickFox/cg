#include "engine/camera.h"

#include <glm/glm.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glog/logging.h>

namespace engine {

glm::mat4 Camera::GetProjectMatrix() const {
  if (type_ == Type::Perspective) {
    return glm::perspective(glm::radians(perspective_fov_), aspect_, near_clip_, far_clip_);
  } else if (type_ == Type::Orthographic) {
    return glm::ortho(-orthographic_width_ / 2, orthographic_width_ / 2,
                      -orthographic_height() / 2, orthographic_height() / 2, near_clip_, far_clip_);
  } else {
    CGCHECK(false) << "Unsupported Camera Type";
    return glm::mat4(1);
  }
}

glm::mat4 Camera::GetViewMatrix() const {
  glm::vec3 center = transform_.translation() + transform_.rotation() * glm::vec3(0, 0, -1);
  return glm::lookAt(transform_.translation(), center, world_up_);
}

void Camera::MoveForward(float delta) {
  transform_.Translate(delta * front());
}

void Camera::MoveRight(float delta) {
  transform_.Translate(delta * right());
}

void Camera::RotateHorizontal(float delta) {
  glm::quat q = glm::angleAxis(-delta, world_up_);
  transform_.Rotate(q);
}

void Camera::RotateVerticle(float delta) {
  glm::quat q = glm::angleAxis(-delta, right());
  transform_.Rotate(q);
}

}