#pragma once

#include "engine/camera.h"
#include "engine/color_frame_buffer.h"
#include "engine/ms_frame_buffer.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "engine/transform.h"
#include "playground/context.h"
#include "playground/object/cube.h"
#include "playground/object/fullscreen_quad.h"
#include "playground/scene.h"

// TODO : I don't know why cannot use
class AATestScene : public Scene {
 public:
  void OnEnter(Context* context);
  void OnUpdate(Context* context);
  void OnGui(Context* context);
  void OnRender(Context* contexnt);
  void OnExit(Context* context);

 private:
  static constexpr int kCubeNum = 1; 
  glm::vec3 cube_positions_[kCubeNum];

  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();
  std::vector<std::unique_ptr<Cube>> cubes_;
  engine::ColorFrameBuffer color_frame_buffer_;
  engine::MSFrameBuffer ms_frame_buffer_;
};