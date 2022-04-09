#pragma once

#include "playground/context.h"

class OnUpdateCommon {
 public:
  OnUpdateCommon(Context* context, const std::string& title);
  ~OnUpdateCommon();

 private:
  void GuiFps(Context* context);
  void GuiCamera(Context* context);
  void ControlCameraByIo(Context* context);
};