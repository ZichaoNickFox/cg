#pragma once

#include "renderer/camera.h"
#include "renderer/definition.h"
#include "renderer/scene.h"
#include "renderer/shader.h"
#include "renderer/ssbo.h"
#include "renderer/texture.h"

class SampleScene : public cg::Scene {
 public:
  SampleScene() : samples_ssbo_(SSBO_USER_0) {}
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

  cg::ObjectMeta object_metas_ = { "sphere", {}, "sphere", "gold"};

 private:
  cg::SSBO samples_ssbo_;
};