#pragma once

#include <unordered_map>

#include "renderer/automic_counter.h"
#include "renderer/BVH.h"
#include "renderer/camera.h"
#include "renderer/definition.h"
#include "renderer/geometry.h"
#include "renderer/object.h"
#include "renderer/primitive.h"
#include "renderer/scene.h"
#include "renderer/ssbo.h"

class PathTracingScene : public renderer::Scene {
 public:
  PathTracingScene() : light_path_ssbo_(SSBO_LIGHT_PATH) {}
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  void Rasterization();
  void PathTracing();

  std::vector<renderer::ObjectMeta> object_metas_ = {
    {"cornell_box_floor", {glm::vec3(), glm::quat(), glm::vec3(0.002, 0.002, 0.002)},
        "cornell_box_floor", "cornell_box_floor"},
    {"cornell_box_left", {glm::vec3(), glm::quat(), glm::vec3(0.002, 0.002, 0.002)},
        "cornell_box_left", "cornell_box_left"},
    {"cornell_box_light", {glm::vec3(), glm::quat(), glm::vec3(0.002, 0.002, 0.002)},
        "cornell_box_light", "cornell_box_light"},
    {"cornell_box_right", {glm::vec3(), glm::quat(), glm::vec3(0.002, 0.002, 0.002)},
        "cornell_box_right", "cornell_box_right"},
    {"cornell_box_short_box", {glm::vec3(), glm::quat(), glm::vec3(0.002, 0.002, 0.002)},
        "cornell_box_short_box", "cornell_box_short_box"},
    {"cornell_box_tall_box", {glm::vec3(), glm::quat(), glm::vec3(0.002, 0.002, 0.002)},
        "cornell_box_tall_box", "cornell_box_tall_box"},
    {"sphere", renderer::Transform(), "sphere", "sphere"},
    {"coordinator", renderer::Transform(), "coordinator", "coordinator"}
  };

  renderer::Texture canvas_;
  renderer::SSBO light_path_ssbo_;
};
