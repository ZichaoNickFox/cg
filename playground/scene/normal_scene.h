#pragma once

#include <memory>

#include "engine/camera.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/cube.h"
#include "playground/object/lines.h"
#include "playground/object/plane.h"
#include "playground/object/point_light.h"
#include "playground/object/sphere.h"
#include "playground/scene.h"
#include "playground/shaders.h"

class NormalScene : public Scene {
 public:
  void OnEnter(Context* context);
  void OnUpdate(Context* context);
  void OnRender(Context* contexnt);
  void OnExit(Context* context);

 private:
  glm::vec4 light_color_ = glm::vec4(1, 1, 1, 1);

  bool use_blinn_phong_ = false;
  bool show_vertex_normal_ = false;
  bool show_texture_normal_ = false;
  bool show_TBN_ = false;
  bool show_triangle_ = false;
  float shininess_ = 0.0;

  PointLight point_light_;
  Plane plane_;
  Sphere sphere_;
  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();

  std::string material_property_name_ = "gold";

  Coord coord_;

  glm::vec3 kLineFrom = glm::vec3(0.8, 1, 0.8);
  glm::vec3 kLineTo = glm::vec3(0.1, -1, 0.1);
  Lines line_;
  std::unique_ptr<Lines> intersect_line_;
};