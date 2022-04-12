#pragma once

#include "engine/camera.h"
#include "engine/material.h"
#include "engine/mesh.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "engine/transform.h"
#include "playground/context.h"
#include "playground/object/lines.h"

class Coord : public Object {
 public:
  Coord();
  void OnUpdate(Context *context) override;
  void OnRender(Context *context) override;
  void OnDestory(Context *context) override;

  int material_num() const override { return 0; }
  engine::Material* mutable_material(int index = 0) override { return nullptr; }

 private: 
  Lines lines_;
};