#pragma once

#include "engine/camera.h"
#include "engine/material.h"
#include "engine/mesh.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "engine/transform.h"
#include "playground/context.h"
#include "playground/object/lines.h"
#include "playground/object/object.h"

class Cube : public Object {
 public:
  void OnUpdate(Context *context) override;
  void OnRender(Context *context) override;
  void OnDestory(Context *context) override;

  int material_num() const override { return 1; }
  engine::Material* mutable_material(int index = 0) override { return &material_; }

  std::shared_ptr<const engine::Mesh> mesh(Context* context) const override { return context->GetMesh("cube"); }

 private: 
  std::shared_ptr<const engine::Mesh> mesh_;
  engine::Material material_;
};