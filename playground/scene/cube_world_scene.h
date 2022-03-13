#pragma once

#include "engine/camera.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/scene.h"

class CubeWorldScene : public Scene {
 public:
  void OnEnter(Context* context);
  void OnUpdate(Context* context);
  void OnGui(Context* context);
  void OnRender(Context* contexnt);
  void OnExit(Context* context);

 private:
  GLuint vao_;
  GLuint vbo_;
  std::shared_ptr<engine::Texture> texture0_;
  std::shared_ptr<engine::Texture> texture1_;
  std::shared_ptr<engine::Shader> shader_;

  glm::vec3 cube_positions_[10];
};