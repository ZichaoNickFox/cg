#pragma once

#include "engine/camera.h"
#include "engine/material.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "engine/transform.h"
#include "playground/context.h"
#include "playground/object/lines.h"
#include "playground/object/object.h"

class PointLight : public Object {
 public:
  PointLight();
  void OnUpdate(Context *context) override;
  void OnRender(Context *context) override;
  void OnDestory(Context *context) override;

  void SetMaterial(const engine::Material& material) { material_ = material; }
  engine::Material* mutable_material() { return &material_; }

 private: 
  GLuint vao_;
  GLuint vbo_;

  engine::Material material_;
};