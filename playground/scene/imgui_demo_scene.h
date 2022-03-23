#pragma once

#include "playground/context.h"
#include "playground/scene.h"

class ImGuiDemoScene : public Scene {
 public:
  void OnEnter(Context* context) override;
  void OnUpdate(Context* context) override;
  void OnGui(Context* context) override;
  void OnRender(Context* context) override;
  void OnExit(Context* context) override;
};