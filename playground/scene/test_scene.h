#pragma once

#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/scene.h"

class TestScene : public Scene {
 public:
  void OnEnter(Context* context);
  void OnUpdate(Context* context);
  void OnGui(Context* context);
  void OnRender(Context* contexnt);
  void OnExit(Context* context);

 private:
  GLuint vao_;
  GLuint vbo_;
  engine::Texture texture0_;
  engine::Texture texture1_;
  engine::Shader shader_;
};