#pragma once

#include <functional>

#include "playground/context.h"

class Scene {
 public:
  virtual ~Scene() {}
  virtual void OnEnter(Context* context) = 0;
  virtual void OnUpdate(Context* context) = 0;
  virtual void OnRender(Context* context) = 0;
  virtual void OnExit(Context* context) = 0;
};