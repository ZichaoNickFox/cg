#pragma once

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "engine/transform.h"

namespace engine {

class Camera {
 public:
  enum Type {
    Perspective,
    Orthographic
  };
  void SetPerspectiveFov(float perspective_fov) { perspective_fov_ = perspective_fov; }
  float perspective_fov() const { return perspective_fov_; }
  void SetPerspectiveAspect(float perspective_aspect) { perspective_aspect_ = perspective_aspect; }
  float perspective_aspect() const { return perspective_aspect_; }

  void SetOrthographicWidth(float width) { orthographic_width_ = width; }
  void SetOrthographicHeight(float height) { orthographic_height_ = height; }

  void SetNearClip(float in) { near_clip_ = in; }
  float nearClip() const { return near_clip_; }
  void SetFarClip(float in) { far_clip_ = in; }
  float farClip() const { return far_clip_; }
  void SetType(Type type) { type_ = type; }
  Type type() const { return type_; }

  void SetTransform(const Transform& transform) { transform_ = transform; }
  engine::Transform* mutable_transform() { return &transform_; }
  const Transform& transform() const { return transform_; }

  glm::vec3 front() const { return transform_.rotation() * glm::vec3(0, 0, -1); }
  glm::vec3 right() const { return transform_.rotation() * glm::vec3(1, 0, 0); }

  void MoveForward(float delta);
  void MoveRight(float delta);
  void RotateHorizontal(float delta);
  void RotateVerticle(float delta);

  glm::mat4 GetProjectMatrix() const;
  glm::mat4 GetViewMatrix() const;

private:
  float perspective_fov_ = 30.0;
  float perspective_aspect_ = 1920.0f / 1080.0f;

  float orthographic_width_ = 860;
  float orthographic_height_ = 640;

  float near_clip_ = 0.1;
  float far_clip_ = 250;

  Type type_ = Type::Perspective;

  Transform transform_;
  glm::vec3 right_ = glm::vec3(1, 0, 0);
  glm::vec3 world_up_ = glm::vec3(0, 1, 0);
};

};