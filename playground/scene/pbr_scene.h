#pragma once

#include "engine/camera.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/cube.h"
#include "playground/object/lines.h"
#include "playground/object/model.h"
#include "playground/object/plane.h"
#include "playground/object/point_light.h"
#include "playground/object/skybox.h"
#include "playground/object/sphere.h"
#include "playground/scene.h"
#include "playground/shaders.h"

class PbrScene : public Scene {
 public:
  void OnEnter(Context* context);
  void OnUpdate(Context* context);
  void OnRender(Context* contexnt);
  void OnExit(Context* context);

 private:
  glm::vec4 light_color_ = glm::vec4(1, 1, 1, 1);
  glm::vec3 albedo_ = glm::vec3(1, 0, 0);

  int light_num = 4;
  std::vector<PointLight> point_lights_;
  Sphere sphere_;
  Coord coord_;
  Skybox skybox_;
  Plane plane_;
  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();

  float metallic = 0.5;
  float roughness = 0.5;
  float ao = 1.0;
};