#pragma once

#include <string>

#include "engine/material.h"
#include "engine/transform.h"
#include "playground/context.h"

class Object {
 public:
  virtual ~Object() {}
  virtual void OnUpdate(Context *context) = 0;
  virtual void OnRender(Context *context, int instance_num = 1) = 0;
  virtual void OnDestory(Context *context) = 0;
  virtual int material_num() const = 0;
  virtual engine::Material* mutable_material(int index = 0) = 0;
  virtual std::shared_ptr<const engine::Mesh> mesh(Context* context) const = 0;

  void SetTransform(const engine::Transform& transform) { transform_ = transform; }
  engine::Transform* mutable_transform() { return &transform_; }
  const engine::Transform& transform() const { return transform_; }
  glm::mat4 GetModelMatrix() const { return transform_.GetModelMatrix(); }

  struct IntersectResult {
    glm::vec3 position_ws;
    glm::vec3 normal_ws;
    float distance_ws;
    glm::vec3 vertex0_ws;
    glm::vec3 vertex1_ws;
    glm::vec3 vertex2_ws;
  };
  bool Intersect(Context* context, const glm::vec3& origin_ws, const glm::vec3& dir_ws, IntersectResult* result);

 protected:
  glm::mat4 GetInverseModelMatrix() { return glm::inverse(GetModelMatrix()); }

  engine::Transform transform_;
  std::string name_;
};