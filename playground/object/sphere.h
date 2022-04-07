#pragma once

#include "engine/camera.h"
#include "engine/material.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/object.h"

class Sphere : public Object {
 public:
  void OnUpdate(Context *context);
  void OnRender(Context *context);
  void OnDestory(Context *context);
  
  engine::Material* mutable_material() { return &material_; }

 private: 
  engine::Material material_;
};