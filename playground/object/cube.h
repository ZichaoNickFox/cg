#pragma once

#include "engine/camera.h"
#include "engine/material.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "engine/transform.h"
#include "playground/context.h"

class Cube {
 public:
  Cube();
  void OnUpdate(Context *context);
  void OnRender(Context *context);
  void OnDestory(Context *context);

  void SetMaterial(const engine::Material& material) { material_ = material; }
  void SetTransform(const engine::Transform& transform) { transform_ = transform; }
  engine::Material* mutable_material() { return &material_; }
  engine::Transform* mutable_transform() { return &transform_; }

 private: 
  GLuint vao_;
  GLuint vbo_;

  engine::Material material_;
  engine::Transform transform_;
};