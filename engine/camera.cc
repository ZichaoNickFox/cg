#include "engine/camera.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glog/logging.h>

namespace engine {

glm::mat4 Camera::GetProjectMatrix() const {
  return glm::perspective(glm::radians(fov_), aspect_, near_clip_, far_clip_);
}

glm::mat4 Camera::GetViewMatrix() const {
  return glm::lookAt(position_, position_ + front_, world_up_);
}

void Camera::MoveForward(float delta) {
  position_ += delta * front_;
}

void Camera::MoveRight(float delta) {
  position_ += delta * right_;
}

void Camera::RotateHorizontal(float delta) {
  glm::mat4 rot = glm::rotate(glm::mat4(1), -delta, world_up_);
  front_ = glm::vec3(rot * glm::vec4(front_, 1));
  right_ = glm::cross(front_, world_up_);
}

void Camera::RotateVerticle(float delta) {
  glm::mat4 rot = glm::rotate(glm::mat4(1), -delta, right_);
  glm::vec3 target_front = glm::normalize(glm::vec3(rot * glm::vec4(front_, 1)));
  // front project to y axis need to between 0.95 to -0.95
  if (target_front.y < 0.95 && target_front.y > -0.95) {
    front_ = target_front;
    right_ = glm::cross(front_, world_up_);
  }
}

void Camera::SetFront(const glm::vec3& front) {
  front_ = front;
  right_ = glm::cross(front_, world_up_);
}

void Camera::SetRight(const glm::vec3& right) {
  right_ = right;
  front_ = glm::cross(world_up_, front_);
}

}