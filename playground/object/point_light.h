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

  int material_num() const override { return 1; }
  engine::Material* mutable_material(int index = 0) override { return &material_; }

  void SetColor(const glm::vec3& color) { color_ = color; }
  glm::vec3 color() const { return color_; }

  void SetAttenuationMetre(int metre) { attenuation_metre_ = metre; }
  int attenuation_metre() const { return attenuation_metre_; }

 private: 
  glm::vec3 color_ = glm::vec3(1, 1, 1);

  int attenuation_metre_ = 100; // 7 / 13 / 20 / 32 / 50 / 65 / 100 / 160 / 200 / 325 / 600 / 3250

  engine::Material material_;
};