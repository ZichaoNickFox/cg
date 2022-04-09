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
  void OnUpdate(Context *context) override;
  void OnRender(Context *context) override;
  void OnDestory(Context *context) override;

  void SetMaterial(const engine::Material& material) { material_ = material; }
  engine::Material* mutable_material() { return &material_; }

  void SetColor(const glm::vec3& color) { color_ = color; }
  glm::vec3 color() { return color_; }

  float attenuation_constant() { return attenuation_constant_; }
  float attenuation_linear() { return attenuation_linear_; }
  float attenuation_quadratic() { return attenuation_quadratic_; }

 private: 
  glm::vec3 color_;

  float attenuation_constant_ = 1.0;
  float attenuation_linear_ = 0.7;
  float attenuation_quadratic_ = 1.8;

  engine::Material material_;
};