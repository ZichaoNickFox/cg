#pragma once

#include "engine/camera.h"
#include "engine/material.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/object.h"
#include "playground/shader_repo.h"

class Lines : public Object {
 public:
  struct Data {
    std::vector<glm::vec3> points;
    std::vector<glm::vec3> colors;
    GLuint primitive;    // line_style : GL_LINES, GL_LINE_STRIP, GL_LINE_LOOP
    int line_width = 1;
  };
  void SetData(Context *context, const Data& data);
  void OnUpdate(Context *context);
  void OnRender(Context *context);
  void OnDestory(Context *context);

 private: 
  void OnInit(Context *context, const Data& data);

  engine::Material material_;

  int line_width_;
  int vertex_size_;
  GLuint primitive_;
  GLuint vao_;
  GLuint vbo_;
};