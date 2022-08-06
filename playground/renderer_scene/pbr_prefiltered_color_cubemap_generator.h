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

class PbrPrefilteredColorCubemapGenerator : public Scene {
 public:
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  std::shared_ptr<renderer::Camera> camera_ = std::make_shared<renderer::Camera>();
  CubeObject cube_;

  renderer::Camera cubemap_cameras_[6];
  std::vector<renderer::Transform> transforms_ = {
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


  static constexpr int kLevel0Size = 512;
  static constexpr int kMipmapMaxLevel = 5;
  static constexpr char kCubemapNamePrefix[] = "pbr_prefiltered_color_cubemap_level_";
  renderer::ColorFramebuffer color_framebuffers_[kMipmapMaxLevel];
};