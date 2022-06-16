#pragma once

#include "engine/camera.h"
#include "engine/material.h"
#include "engine/mesh.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "engine/transform.h"
#include "playground/context.h"
#include "playground/object/lines_object.h"
#include "playground/object/object.h"

class SkyboxObject : public Object {
 public:
  void OnUpdate(Context *context) override;
  void OnRender(Context *context, int instance_num = 1) override;
  void OnDestory(Context *context) override;

  int material_num() const override { return 1; }
  engine::Material* mutable_material(int index = 0) override { return &material_; }

  std::shared_ptr<const engine::Mesh> GetMesh(Context* context) const override { return context->GetMesh("cube"); }

 private: 
  engine::Material material_;
};