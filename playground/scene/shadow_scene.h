#pragma once

#include "engine/camera.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/cube.h"
#include "playground/object/lines.h"
#include "playground/object/plane.h"
#include "playground/scene.h"

class ShadowScene : public Scene {
 public:
  void OnEnter(Context* context);
  void OnUpdate(Context* context);
  void OnGui(Context* context);
  void OnRender(Context* contexnt);
  void OnExit(Context* context);

 private:
  struct MaterialProperty {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
  };
  MaterialProperty gold_ = {
    glm::vec4(0.24725, 0.1995, 0.0745, 1),
    glm::vec4(0.75164, 0.60648, 0.22648, 1),
    glm::vec4(0.628281, 0.555802, 0.366065, 1),
    51.2};
  MaterialProperty material_property_ = gold_;
  glm::vec3 light_scale_ = glm::vec3(.2, .2, .2);
  glm::vec3 light_color_ = glm::vec3(1, 1, 1);

  std::vector<Cube> cubes_;
  std::vector<engine::Transform> cube_transforms_;

  Cube light_;
  Lines coord_;
  Plane plane_;
  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();
};
#pragma once

