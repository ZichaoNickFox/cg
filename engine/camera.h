#pragma once

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "engine/transform.h"

namespace engine
{

class Camera {
 public:
  enum Type {
    Perspective,
    Orthographic
  };
  void SetFov(float in) { fov_ = in; }
  float fov() const { return fov_; }
  void SetAspect(float in) { aspect_ = in; }
  float aspect() const { return aspect_; }
  void SetNearClip(float in) { near_clip_ = in; }
  float nearClip() const { return near_clip_; }
  void SetFarClip(float in) { far_clip_ = in; }
  float farClip() const { return far_clip_; }
  void SetType(Type in) { type_ = in; }
  Type type() const { return type_; }

  void SetTransform(const Transform& transform) { transform_ = transform; }
  engine::Transform* mutable_transform() { return &transform_; }
  const Transform& transform() const { return transform_; }
  void SetFront(const glm::vec3& front);
  const glm::vec3& front() const { return front_; }
  void SetRight(const glm::vec3& right);
  const glm::vec3& right() const { return right_; }

  void MoveForward(float delta);
  void MoveRight(float delta);
  void RotateHorizontal(float delta);
  void RotateVerticle(float delta);

  glm::mat4 GetProjectMatrix() const;
  glm::mat4 GetViewMatrix() const;

private:
  float fov_ = 30.0;
  float aspect_ = 1920.0f / 1080.0f;
  float near_clip_ = 0.1;
  float far_clip_ = 250;
  Type type_ = Type::Perspective;

  Transform transform_;
  glm::vec3 front_ = glm::vec3(0 ,0, -1);
  glm::vec3 right_ = glm::vec3(1, 0, 0);
  glm::vec3 world_up_ = glm::vec3(0, 1, 0);
};

};