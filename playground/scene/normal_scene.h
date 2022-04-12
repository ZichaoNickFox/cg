#pragma once

#include "engine/camera.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/coord.h"
#include "playground/object/cube.h"
#include "playground/object/plane.h"
#include "playground/object/point_light.h"
#include "playground/object/sphere.h"
#include "playground/scene.h"
#include "playground/shader_param.h"

class NormalScene : public Scene {
 public:
  void OnEnter(Context* context);
  void OnUpdate(Context* context);
  void OnRender(Context* contexnt);
  void OnExit(Context* context);

 private:
  glm::vec3 light_color_ = glm::vec3(1, 1, 1);

  bool use_blinn_phong_ = false;

  PointLight point_light_;
  Plane plane_;
  Sphere sphere_;
  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();

  std::string material_property_name_ = "gold";

  Coord coord_;
};