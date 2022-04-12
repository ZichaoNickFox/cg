#pragma once

#include "playground/object/plane.h"
#include "playground/object/object.h"

class Billboard : public Object {
 public:
  struct Data {
    std::string texture;
  };
  void Init(Context *context, const Data& data);
  void OnUpdate(Context *context) override;
  void OnRender(Context *context) override;
  void OnDestory(Context *context) override;

  int material_num() const override { return 0; }
  engine::Material* mutable_material(int index = 0) override { return nullptr; }

 private:
  Plane plane_;
};