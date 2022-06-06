#pragma once

#include "playground/object/plane_object.h"
#include "playground/object/object.h"

class BillboardObject : public PlaneObject {
 public:
  struct Data {
    std::string texture;
  };
  void Init(Context *context, const Data& data);
  void OnUpdate(Context *context) override;
};