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

class Skybox : public Object {
 public:
  void OnUpdate(Context *context) override;
  void OnRender(Context *context) override;
  void OnDestory(Context *context) override;

  int material_num() const override { return 1; }
  engine::Material* mutable_material(int index = 0) override { return &material_; }

  std::shared_ptr<const engine::Mesh> mesh(Context* context) const override { return context->GetMesh("cube"); }

  void SetCubeMap(engine::Texture cube_map);
  
 private: 
  engine::Material material_;
  engine::Texture cube_map_;
  bool has_cube_map_ = false;
};