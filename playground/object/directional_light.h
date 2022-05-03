#pragma once

#include "engine/camera.h"
#include "engine/frame_buffer/depth_frame_buffer.h"
#include "playground/context.h"
#include "playground/object/billboard.h"
#include "playground/object/lines.h"
#include "playground/object/object.h"

class DirectionalLight : public Object {
 public:
  void Init(Context* context);

  void OnUpdate(Context *context) override;
  void OnRender(Context* context) override;
  void OnDestory(Context* context) override;
  int material_num() const override { return 0; }
  engine::Material* mutable_material(int index = 0) override { return nullptr; }
  std::shared_ptr<const engine::Mesh> mesh(Context* context) const override { return nullptr; }

  Billboard* mutable_billboard() { return &billboard_; }
  Lines* mutable_lines() { return &lines_; }

 private:
  static int directional_light_num_;

  Billboard billboard_;
  Lines lines_;
};