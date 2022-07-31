#include "renderer/camera.h"

#include <glm/glm.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/gtx/string_cast.hpp>

#include "base/debug.h"

namespace renderer {

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

// https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluLookAt.xml
// https://zhuanlan.zhihu.com/p/138157800
glm::mat4 LookAt(const glm::vec3& transform, const glm::vec3& center, const glm::vec3& world_up) {
  glm::vec3 front = normalize(center - transform);
  glm::vec3 right = normalize(glm::cross(front, world_up));
  glm::vec3 up = normalize(glm::cross(right, front));
  glm::mat4 translation_v2w = glm::mat4(1, 0, 0, -transform.x,
                                        0, 1, 0, -transform.y,
                                        0, 0, 1, -transform.z,
                                        0, 0, 0, 1);
  // TODO why cannot use inverse?
  glm::mat4 translation_w2v = glm::transpose(translation_v2w);
  glm::mat4 rotation_v2w = glm::mat4(right.x, right.y, right.z, 0,
                                     up.x, up.y, up.z, 0,
                                     -front.x, -front.y, -front.z, 0,
                                     0, 0, 0, 1);
  glm::mat4 rotation_w2v = glm::transpose(rotation_v2w);
  return rotation_w2v * translation_w2v;
}

glm::mat4 Camera::GetViewMatrix() const {
  glm::vec3 center = transform_.translation() + transform_.rotation() * glm::vec3(0, 0, -1);
  return glm::lookAtRH(transform_.translation(), center, world_up_);
  //return LookAt(transform_.translation(), center, world_up_);
}

void Camera::MoveForwardWS(float delta) {
  transform_.Translate(delta * front_ws());
}

void Camera::MoveRightWS(float delta) {
  transform_.Translate(delta * right_ws());
}

void Camera::RotateHorizontal(float delta) {
  glm::quat q = glm::angleAxis(-delta, world_up_);
  transform_.Rotate(q);
}

void Camera::RotateVerticle(float delta) {
  glm::quat q = glm::angleAxis(-delta, right_ws());
  transform_.Rotate(q);
}

// https://feepingcreature.github.io/math.html
void Camera::GetPickRay(const glm::vec2& cursor_pos_ss, glm::vec3* near_position_ws,
                        glm::vec3* far_position_ws) const {
  glm::vec3 near_screen_space = glm::vec3(cursor_pos_ss.x, cursor_pos_ss.y, 0);
  glm::vec3 far_screen_space = glm::vec3(cursor_pos_ss.x, cursor_pos_ss.y, 1);
  std::vector<glm::vec3> screen_spaces{near_screen_space, far_screen_space};
  std::vector<glm::vec4> world_spaces(2);
  for (int i = 0; i < screen_spaces.size(); ++i) {
    const glm::vec3& screen_space = screen_spaces[i];
    glm::vec3 ndc_space = screen_space * 2.0f - 1.0f;
    glm::mat4 inverse_vp = glm::inverse(GetProjectMatrix() * GetViewMatrix());
    world_spaces[i] = inverse_vp * glm::vec4(ndc_space, 1.0f);
    world_spaces[i] = world_spaces[i] / world_spaces[i].w;
  }
  *near_position_ws = world_spaces[0];
  *far_position_ws = world_spaces[1];
}

renderer::Ray Camera::GetPickRay(const glm::vec2& cursor_pos_ss) const {
  glm::vec3 near_position_ws;
  glm::vec3 far_position_ws;
  GetPickRay(cursor_pos_ss, &near_position_ws, &far_position_ws);
  return renderer::Ray{near_position_ws, glm::normalize(far_position_ws - near_position_ws)};
}

}