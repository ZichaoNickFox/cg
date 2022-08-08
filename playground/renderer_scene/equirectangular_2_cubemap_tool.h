#pragma once

#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/glm.hpp>

#include "base/math.h"
#include "renderer/camera.h"
#include "renderer/framebuffer/color_framebuffer.h"
#include "renderer/scene.h"
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "playground/context.h"
#include "playground/object/cube_object.h"
#include "playground/object/empty_object.h"

class Equirectangular2CubemapTool : public Scene {
 public:
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  std::shared_ptr<cg::Camera> camera_ = std::make_shared<cg::Camera>();
  CubeObject cube_;
  CoordObject coord_;

  cg::Camera cubemap_cameras_[6];
  std::vector<cg::Transform> transforms_ = {
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

  cg::ColorFramebuffer color_framebuffer_;
};