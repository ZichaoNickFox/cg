#pragma once

#include "engine/camera.h"
#include "engine/framebuffer/color_framebuffer.h"
#include "engine/framebuffer/ms_framebuffer.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "engine/transform.h"
#include "playground/context.h"
#include "playground/object/cube_object.h"
#include "playground/object/empty_object.h"
#include "playground/scene.h"

// TODO : I don't know why cannot use
class AATestScene : public Scene {
 public:
  void OnEnter(Context* context);
  void OnUpdate(Context* context);
  void OnRender(Context* contexnt);
  void OnExit(Context* context);

 private:
  static constexpr int kCubeNum = 1; 
  glm::vec3 cube_positions_[kCubeNum];

  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();
  std::vector<std::unique_ptr<CubeObject>> cubes_;
  engine::ColorFramebuffer color_framebuffer_;
  engine::MSFramebuffer ms_framebuffer_;
};