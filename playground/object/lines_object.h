#pragma once

#include "engine/camera.h"
#include "engine/material.h"
#include "engine/repo/shader_repo.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/object.h"

class LinesObject : public Object {
 public:
  struct Mesh {
    std::vector<glm::vec3> points;
    std::vector<glm::vec3> colors;
    GLuint primitive_mode;    // line_style : GL_LINES, GL_LINE_STRIP, GL_LINE_LOOP
  };
  void SetMesh(const Mesh& data);

  void OnUpdate(Context *context) override;
  void OnRender(Context *context, int instance_num = 1) override;
  void OnDestory(Context *context) override;
  
  int material_num() const override { return 1; }
  engine::Material* mutable_material(int index = 0) override { return &material_; }

  std::shared_ptr<const engine::Mesh> mesh(Context* context) const override { return nullptr; }

 private: 
  void OnInit(const Mesh& data);
  void Clear();

  engine::Material material_;

  int vertex_size_;
  GLuint primitive_mode_;
  GLuint vao_;
  GLuint vbo_;
};

class Coord : public LinesObject {
 public:
  Coord();
};