#pragma once

#include <string>

#include "engine/transform.h"

class Object {
 public:
  void SetTransform(const engine::Transform& transform) { transform_ = transform; }
  engine::Transform* mutable_transform() { return &transform_; }
  const engine::Transform& transform() { return transform_; }

 protected:
  engine::Transform transform_;

  std::string name_;
};