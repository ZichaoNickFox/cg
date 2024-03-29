#pragma once

#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

#include "renderer/camera.h"
#include "renderer/light.h"
#include "renderer/scene.h"
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "renderer/transform.h"

class ModelScene : public cg::Scene {
 public:
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  std::vector<cg::ObjectMeta> object_metas_ = {
    // {"teapot", {glm::vec3(), glm::quat(), glm::vec3(0.2, 0.2, 0.2)}, "teapot", "gold"},
    {"cerberus", {glm::vec3(), glm::quat(), glm::vec3(0.02, 0.02, 0.02)}, "cerberus", "cerberus"}
  };
  std::vector<cg::Light> lights = {
    {cg::Light::kPointLight, glm::vec3(0, 3, -5), glm::vec4(1, 1, 1, 1), {1.0, 0.0014, 0.000007}},
  };

  float shininess_ = 1;
  bool use_texture_ambient_ = true;
  bool use_texture_normal_ = true;
  bool use_texture_specular_ = true;
  bool use_texture_diffuse_ = true;
  bool use_texture_height_ = true;
  bool show_vertex_normal_ = false;
  bool show_texture_normal_ = false;
  bool show_TBN_ = false;
  bool show_triangle_ = false;
  bool use_blinn_phong_ = false;

  float rotate_speed_ = 0.01;

  bool enable_cull_face_ = false;
  int call_face_ = GL_BACK;
  int cw_ = GL_CW;
};