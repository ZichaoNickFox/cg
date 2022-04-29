#pragma once

#include "engine/camera.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/cube.h"
#include "playground/object/lines.h"
#include "playground/object/model.h"
#include "playground/object/plane.h"
#include "playground/object/point_light.h"
#include "playground/object/skybox.h"
#include "playground/object/sphere.h"
#include "playground/scene.h"
#include "playground/shaders.h"

class SampleScene : public Scene {
 public:
  void OnEnter(Context* context);
  void OnUpdate(Context* context);
  void OnRender(Context* contexnt);
  void OnExit(Context* context);

 private:
  Sphere sphere_;
  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();
};