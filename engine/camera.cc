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

// https://feepingcreature.github.io/math.html
void Camera::GetPickRay(const glm::vec2& normalized_cursor_screen_pos, glm::vec3* cursor_world_pos_near,
                        glm::vec3* cursor_world_pos_far) const {
  glm::vec3 near_screen_space = glm::vec3(normalized_cursor_screen_pos.x, normalized_cursor_screen_pos.y, 0);
  glm::vec3 far_screen_space = glm::vec3(normalized_cursor_screen_pos.x, normalized_cursor_screen_pos.y, 1);
  std::vector<glm::vec3> screen_spaces{near_screen_space, far_screen_space};
  std::vector<glm::vec4> world_spaces(2);
  for (int i = 0; i < screen_spaces.size(); ++i) {
    const glm::vec3& screen_space = screen_spaces[i];
    glm::vec3 ndc_space = screen_space * 2.0f - 1.0f;
    glm::mat4 inverse_vp = glm::inverse(GetProjectMatrix() * GetViewMatrix());
    world_spaces[i] = inverse_vp * glm::vec4(ndc_space, 1.0f);
    world_spaces[i] = world_spaces[i] / world_spaces[i].w;
  }
  *cursor_world_pos_near = world_spaces[0];
  *cursor_world_pos_far = world_spaces[1];
}

}