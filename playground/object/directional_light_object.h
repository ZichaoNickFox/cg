#pragma once

#include "engine/camera.h"
#include "playground/context.h"
#include "playground/object/billboard_object.h"
#include "playground/object/lines_object.h"
#include "playground/object/object.h"

class DirectionalLightObject : public Object {
 public:
  void Init(Context* context);

  void OnUpdate(Context* context) override;
  void OnRender(Context* context, int instance_num = 1) override;
  void OnDestory(Context* context) override;
  int material_num() const override { return 0; }
  engine::Material* mutable_material(int index = 0) override { return nullptr; }
  std::shared_ptr<const engine::Mesh> GetMesh(Context* context) const override { return nullptr; }

  BillboardObject* mutable_billboard() { return &billboard_; }
  LinesObject* mutable_lines() { return &lines_; }

 private:
  static int directional_light_num_;

  BillboardObject billboard_;
  LinesObject lines_;
};