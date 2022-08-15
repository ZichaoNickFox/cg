#pragma once

#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/glm.hpp>

#include "renderer/camera.h"
#include "renderer/framebuffer/color_framebuffer.h"
#include "renderer/scene.h"
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "playground/context.h"
#include "playground/object/cube_object.h"
#include "playground/object/empty_object.h"

class PbrBRDFIntegrationMapGenerator : public Scene {
 public:
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  std::shared_ptr<cg::Camera> camera_ = std::make_shared<cg::Camera>();
  cg::ColorFramebuffer color_framebuffer_;
};