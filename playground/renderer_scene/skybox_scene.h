#pragma once

#include "renderer/camera.h"
#include "renderer/scene.h"
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "playground/context.h"
#include "playground/object/cube_object.h"
#include "playground/object/lines_object.h"
#include "playground/object/skybox_object.h"

class SkyboxScene : public Scene {
 public:
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  CoordObject coord_;
  SkyboxObject skybox_;
  std::shared_ptr<cg::Camera> camera_ = std::make_shared<cg::Camera>();
};
#pragma once

