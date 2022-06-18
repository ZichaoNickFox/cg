#pragma once

#include "engine/camera.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/cube_object.h"
#include "playground/object/lines_object.h"
#include "playground/object/skybox_object.h"
#include "playground/scene.h"

class SkyboxScene : public Scene {
 public:
  void OnEnter(Context* context);
  void OnUpdate(Context* context);
  void OnRender(Context* contexnt);
  void OnExit(Context* context);

 private:
  CoordObject coord_;
  SkyboxObject skybox_;
  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();
};
#pragma once

