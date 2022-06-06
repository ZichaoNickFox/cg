#pragma once

#include <limits>

#include "engine/camera.h"
#include "engine/framebuffer.h"
#include "engine/material.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "engine/transform.h"
#include "playground/context.h"
#include "playground/object/lines_object.h"
#include "playground/object/object.h"

class EmptyObject : public Object {
 public:
  EmptyObject();
  void OnUpdate(Context *context) override;
  void OnRender(Context *context, int instance_num = 1) override;
  void OnDestory(Context *context) override;

  int material_num() const override { return 1; }
  engine::Material* mutable_material(int index = 0) override { return &material_; }

  std::shared_ptr<const engine::Mesh> mesh(Context* context) const override { return nullptr; }

 private: 
  engine::Material material_;
  GLuint empty_vao_;
};