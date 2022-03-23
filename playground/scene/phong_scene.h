#pragma once

#include "engine/camera.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/cube.h"
#include "playground/scene.h"

class PhongScene : public Scene {
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
  MaterialProperty silver_ = {
    glm::vec4(0.19225, 0.19225, 0.19225, 1),
    glm::vec4(0.50754, 0.50754, 0.50754, 1),
    glm::vec4(0.508273, 0.508273, 0.508273, 1),
    51.2};
  MaterialProperty jade_ = {
    glm::vec4(0.135, 0.2225, 0.1575, 0.95),
    glm::vec4(0.54, 0.89, 0.63, 0.95),
    glm::vec4(0.316228, 0.316228, 0.316228, 0.95),
    12.8};
  MaterialProperty ruby_ = {
    glm::vec4(0.1745, 0.01175, 0.01175, 0.55),
    glm::vec4(0.61424, 0.04136, 0.04136, 0.55),
    glm::vec4(0.727811, 0.626959, 0.626959, 0.55),
    76.8};

  MaterialProperty material_property_ = gold_;
  glm::vec3 light_scale_ = glm::vec3(.2, .2, .2);
  glm::vec3 light_color_ = glm::vec3(1, 1, 1);

  Cube cube_;
  Cube light_;
  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();
};