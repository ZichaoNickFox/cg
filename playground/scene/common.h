#pragma once

#include "playground/context.h"

class OnUpdateCommon {
 public:
  OnUpdateCommon(Context* context, const std::string& title);
  ~OnUpdateCommon();

 private:
  void GuiFps(Context* context);
  void InspectCamera(Context* context);
  void MoveCamera(Context* context);
};