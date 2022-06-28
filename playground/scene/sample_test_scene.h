#pragma once

#include "renderer/camera.h"
#include "renderer/scene.h"
#include "renderer/shader.h"
#include "renderer/ssbo.h"
#include "renderer/texture.h"
#include "playground/context.h"
#include "playground/object/sphere_object.h"

class SampleTestScene : public Scene {
 public:
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  SphereObject sphere_object_;
  std::shared_ptr<renderer::Camera> camera_ = std::make_shared<renderer::Camera>();
  renderer::SSBO samples_ssbo_;
};