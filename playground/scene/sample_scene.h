#pragma once

#include "renderer/camera.h"
#include "renderer/definition.h"
#include "renderer/scene.h"
#include "renderer/shader.h"
#include "renderer/ssbo.h"
#include "renderer/texture.h"

class SampleScene : public renderer::Scene {
 public:
  SampleScene() : samples_ssbo_(SSBO_USER_0) {}
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

  renderer::ObjectMeta object_metas_ = { "sphere", {}, "sphere", "gold"};

 private:
  renderer::SSBO samples_ssbo_;
};