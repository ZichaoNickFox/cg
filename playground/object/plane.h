#pragma once

#include "engine/camera.h"
#include "engine/material.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/object.h"

class Plane : public Object {
 public:
  Plane();
  void OnUpdate(Context *context);
  void OnRender(Context *context);
  void OnDestory(Context *context);

  void SetMaterial(const engine::Material& material) { material_ = material; }
  engine::Material* mutable_material() { return &material_; }

 private: 
  GLuint vao_;
  GLuint vbo_;

  engine::Material material_;

  std::string name_;
};