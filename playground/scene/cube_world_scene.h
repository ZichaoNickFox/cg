#pragma once

#include "engine/camera.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "engine/transform.h"
#include "playground/context.h"
#include "playground/object/cube.h"
#include "playground/scene.h"

class CubeWorldScene : public Scene {
 public:
  void OnEnter(Context* context);
  void OnUpdate(Context* context);
  void OnRender(Context* contexnt);
  void OnExit(Context* context);

 private:
  static constexpr int kCubeNum = 10; 
  glm::vec3 cube_positions_[kCubeNum];

  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();
  std::vector<std::unique_ptr<Cube>> cubes_;
};