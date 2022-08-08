#pragma once

#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/glm.hpp>

#include "base/math.h"
#include "physics/leap_frog.h"
#include "renderer/camera.h"
#include "renderer/framebuffer.h"
#include "renderer/framebuffer_attachment.h"
#include "renderer/object.h"
#include "renderer/scene.h"
#include "renderer/shader.h"
#include "renderer/texture.h"

// https://games103.games-cn.org/HW1/lab1.pdf
class AngryBunnyScene : public cg::Scene {
 public:
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  std::vector<cg::ObjectMeta> object_metas_ = {
    {"bunny", {}, "bunny", "ruby"},
    {"light", {glm::vec3(), glm::quat(), glm::vec3(0.02, 0.02, 0.02)}, "sphere", "gold"},
    {"plane1", {glm::vec3(), glm::quat(), glm::vec3(5, 5, 5)}, "plane", "tiled"},
    {"plane2", {glm::vec3(1, 0, 0), glm::angleAxis(float(M_PI / 2), glm::vec3(0, 0, 1)),
                glm::vec3(5, 5, 5)}, "plane", "tiled"}
  };
};
