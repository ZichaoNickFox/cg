#pragma once

#include "playground/object/plane.h"
#include "playground/object/object.h"

class Billboard : public Plane {
 public:
  struct Data {
    std::string texture;
  };
  void Init(Context *context, const Data& data);
  void OnUpdate(Context *context) override;
};