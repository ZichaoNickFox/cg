#pragma once

#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/glm.hpp>

#include "engine/camera.h"
#include "engine/framebuffer/color_framebuffer.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/cube.h"
#include "playground/object/empty_object.h"
#include "playground/scene.h"
#include "playground/shaders.h"

class PbrIrradianceCubemapGenerator : public Scene {
 public:
  void OnEnter(Context* context);
  void OnUpdate(Context* context);
  void OnRender(Context* contexnt);
  void OnExit(Context* context);

 private:
  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();
  Cube cube_;

  engine::Camera cubemap_cameras_[6];
  std::vector<engine::Transform> transforms_ = {
    // px image : camera looks nx, stand in (1, 0, 0)
    {glm::vec3(1, 0, 0), glm::angleAxis(float(M_PI) / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(1, 1, 1)},
    // nx image : camera looks px, stand in (-1, 0, 0)
    {glm::vec3(-1, 0, 0), glm::angleAxis(float(M_PI) / 2.0f, glm::vec3(0.0f, -1.0f, 0.0f)), glm::vec3(1, 1, 1)},
    // py image : camera looks ny, stand in (0, 1, 0)
    {glm::vec3(0, 1, 0), glm::angleAxis(float(M_PI) / 2.0f, glm::vec3(-1.0f, 0.0f, 0.0f)), glm::vec3(1, 1, 1)},
    // ny image : camera looks py, stand in (0, -1, 0)
    {glm::vec3(0, -1, 0), glm::angleAxis(float(M_PI) / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f)), glm::vec3(1, 1, 1)},
    // pz image : camera looks nz, stand in (0, 0, 1)
    {glm::vec3(0, 0, 1), glm::quat(), glm::vec3(1, 1, 1)},
    // nz image : camera looks pz, stand in (0, 0, -1)
    {glm::vec3(0, 0, -1), glm::angleAxis(float(M_PI), glm::vec3(0, 1, 0)), glm::vec3(1, 1, 1)},
  };

  engine::ColorFramebuffer color_framebuffer_;
};