#pragma once

#include "engine/camera.h"
#include "engine/material.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/object.h"

class Plane : public Object {
 public:
  void OnUpdate(Context *context) override;
  void OnRender(Context *context) override;
  void OnDestory(Context *context) override;
  
  int material_num() const override { return 1; }
  engine::Material* mutable_material(int index = 0) override { return &material_; }

 private: 
  engine::Material material_;
};