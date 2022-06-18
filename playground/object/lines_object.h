#pragma once

#include "engine/camera.h"
#include "engine/geometry.h"
#include "engine/material.h"
#include "engine/repo/shader_repo.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/object.h"

class LinesObject : public Object {
 public:
  struct Mesh {
    Mesh(const std::vector<glm::vec4>& points, const std::vector<glm::vec4>& colors, GLuint primitive_mode);
    Mesh(const std::vector<engine::AABB>& aabbs, const glm::vec4& default_color);
    Mesh(const engine::Triangle& triangle, const glm::vec4& color);
    std::vector<glm::vec4> points;
    std::vector<glm::vec4> colors;
    GLuint primitive_mode;    // line_style : GL_LINES, GL_LINE_STRIP, GL_LINE_LOOP
  };
  void SetMesh(const Mesh& data);

  void OnUpdate(Context *context) override;
  void OnRender(Context *context, int instance_num = 1) override;
  void OnDestory(Context *context) override;
  
  int material_num() const override { return 1; }
  engine::Material* mutable_material(int index = 0) override { return &material_; }

  std::shared_ptr<const engine::Mesh> GetMesh(Context* context) const override { return nullptr; }

 private: 
  void OnInit(const Mesh& data);
  void Clear();

  engine::Material material_;

  int vertex_size_;
  GLuint primitive_mode_;
  GLuint vao_;
  GLuint vbo_;
};

class CoordObject : public LinesObject {
 public:
  CoordObject();
};