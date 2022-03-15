#pragma once

#include "engine/camera.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/cube.h"
#include "playground/scene.h"

class PhongScene : public Scene {
 public:
  void OnEnter(Context* context);
  void OnUpdate(Context* context);
  void OnGui(Context* context);
  void OnRender(Context* contexnt);
  void OnExit(Context* context);

 private:
  glm::vec3 light_scale_ = glm::vec3(.2, .2, .2);
  glm::vec3 light_color_ = glm::vec3(1, 1, 1);

  Cube cube_;
  Cube light_;
};