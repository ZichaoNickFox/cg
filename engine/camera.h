#pragma once

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

  void SetPosition(const glm::vec3& position) { position_ = position; }
  const glm::vec3& position() const { return position_; }
  void SetFront(const glm::vec3& front);
  const glm::vec3& front() const { return front_; }
  void SetRight(const glm::vec3& right);
  const glm::vec3& right() const { return right_; }

  void MoveForward(float delta);
  void MoveRight(float delta);
  void RotateHorizontal(float delta);
  void RotateVerticle(float delta);

  glm::mat4 GetProjectMatrix();
  glm::mat4 GetViewMatrix();

private:
  float fov_ = 30.0;
  float aspect_ = 1920.0f / 1080.0f;
  float near_clip_ = 0.1;
  float far_clip_ = 100;
  Type type_ = Type::Perspective;

  glm::vec3 position_ = glm::vec3(0, 0, 0);
  glm::vec3 front_ = glm::vec3(0 ,0, -1);
  glm::vec3 right_ = glm::vec3(1, 0, 0);
  glm::vec3 world_up_ = glm::vec3(0, 1, 0);
};

};