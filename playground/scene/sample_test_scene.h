#pragma once

#include "engine/camera.h"
#include "engine/shader.h"
#include "engine/SSBO.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/sphere_object.h"
#include "playground/scene.h"
#include "playground/shaders.h"

class SampleTestScene : public Scene {
 public:
  void OnEnter(Context* context);
  void OnUpdate(Context* context);
  void OnRender(Context* contexnt);
  void OnExit(Context* context);

 private:
  SphereObject sphere_object_;
  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();
  engine::SSBO samples_ssbo_;
};