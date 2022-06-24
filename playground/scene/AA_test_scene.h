#pragma once

#include "renderer/camera.h"
#include "renderer/framebuffer/color_framebuffer.h"
#include "renderer/framebuffer/ms_framebuffer.h"
#include "renderer/scene.h"
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "renderer/transform.h"
#include "playground/context.h"
#include "playground/object/cube_object.h"
#include "playground/object/empty_object.h"

// TODO : I don't know why cannot use
class AATestScene : public Scene {
 public:
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  static constexpr int kCubeNum = 1; 
  glm::vec3 cube_positions_[kCubeNum];

  std::shared_ptr<renderer::Camera> camera_ = std::make_shared<renderer::Camera>();
  std::vector<std::unique_ptr<CubeObject>> cubes_;
  renderer::ColorFramebuffer color_framebuffer_;
  renderer::MSFramebuffer ms_framebuffer_;
};