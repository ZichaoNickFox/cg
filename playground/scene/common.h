#pragma once

#include "playground/context.h"

// Put in OnUpdate before everything
class OnUpdateCommon {
 public:
  OnUpdateCommon(Context* context, const std::string& title);
  ~OnUpdateCommon();

 private:
  void GuiFps(Context* context);
  void InspectCamera(Context* context);
  void MoveCamera(Context* context);
  void InSpectCursor(Context* context);
  void ReloadShaders(Context* context);
};

// Put in OnRender after everything
class OnRenderCommon {
 public:
  OnRenderCommon(Context* context);

 private:
  void DrawWorldCoordAndViewCoord(Context* context);
};