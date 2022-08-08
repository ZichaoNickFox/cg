#pragma once

#include <memory>

#include "renderer/camera.h"
#include "renderer/scene.h"
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "playground/context.h"
#include "playground/object/cube_object.h"
#include "playground/object/lines_object.h"
#include "playground/object/plane_object.h"
#include "playground/object/point_light_object.h"
#include "playground/object/sphere_object.h"

class NormalScene : public Scene {
 public:
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  glm::vec4 light_color_ = glm::vec4(1, 1, 1, 1);

  bool use_blinn_phong_ = false;
  bool show_vertex_normal_ = false;
  bool show_texture_normal_ = false;
  bool show_TBN_ = false;
  bool show_triangle_ = false;
  float shininess_ = 0.0;

  PointLightObject point_light_;
  PlaneObject plane_;
  SphereObject sphere_;
  std::shared_ptr<cg::Camera> camera_ = std::make_shared<cg::Camera>();

  std::string material_property_name_ = "gold";

  CoordObject coord_;

  glm::vec3 kLineFrom = glm::vec3(0.8, 1, 0.8);
  glm::vec3 kLineTo = glm::vec3(0.1, -1, 0.1);
  LinesObject line_;
  std::unique_ptr<LinesObject> intersect_line_;
};