#pragma once

#include <string>

#include "engine/material.h"
#include "engine/transform.h"
#include "playground/context.h"

class Object {
 public:
  virtual ~Object() {}
  virtual void OnUpdate(Context *context) = 0;
  virtual void OnRender(Context *context) = 0;
  virtual void OnDestory(Context *context) = 0;
  virtual int material_num() const = 0;
  virtual engine::Material* mutable_material(int index = 0) = 0;

  void SetTransform(const engine::Transform& transform) { transform_ = transform; }
  engine::Transform* mutable_transform() { return &transform_; }
  const engine::Transform& transform() const { return transform_; }
  glm::mat4 GetModelMatrix() const { return transform_.GetModelMatrix(); }

 protected:
  engine::Transform transform_;
  std::string name_;
};