#pragma once

#include "engine/camera.h"
#include "engine/material.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"

class Cube {
 public:
  struct Option {
    engine::Material material;
    glm::vec3 position = glm::vec3(0, 0, 0);
    glm::vec3 scale = glm::vec3(1, 1, 1);
  };
  void Init(const Option& option, Context *context);
  void OnUpdate(Context *context);
  void OnRender(Context *context);
  void OnDestory(Context *context);

 private: 
  GLuint vao_;
  GLuint vbo_;

  Option option_;
};