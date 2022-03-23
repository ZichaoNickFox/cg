#pragma once

#include "engine/camera.h"
#include "engine/material.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "engine/transform.h"
#include "playground/context.h"
#include "playground/object/lines.h"
#include "playground/object/object.h"

class Cube : public Object {
 public:
  Cube();
  void OnUpdate(Context *context);
  void OnRender(Context *context);
  void OnDestory(Context *context);

  void SetMaterial(const engine::Material& material) { material_ = material; }
  engine::Material* mutable_material() { return &material_; }

 private: 
  GLuint vao_;
  GLuint vbo_;

  engine::Material material_;
};