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

 private:
  Plane plane_;
};