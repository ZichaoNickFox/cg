#pragma once

#include "engine/camera.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/cube.h"
#include "playground/object/lines.h"
#include "playground/object/skybox.h"
#include "playground/scene.h"

class SkyboxScene : public Scene {
 public:
  void OnEnter(Context* context);
  void OnUpdate(Context* context);
  void OnRender(Context* contexnt);
  void OnExit(Context* context);

 private:
  Coord coord_;
  Skybox skybox_;
  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();
};
#pragma once

