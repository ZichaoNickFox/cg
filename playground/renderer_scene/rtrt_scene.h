#pragma once

#include <unordered_map>

#include "base/geometry.h"
#include "renderer/automic_counter.h"
#include "renderer/BVH.h"
#include "renderer/camera.h"
#include "renderer/definition.h"
#include "renderer/framebuffer.h"
#include "renderer/object.h"
#include "renderer/ping_pong.h"
#include "renderer/primitive.h"
#include "renderer/scene.h"
#include "renderer/ssbo.h"

class RTRTScene : public cg::Scene {
 public:
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  void Rasterization();

  void PathTracing();
  void OutlierClamping();
  void Denoise();
  void TemproalAccumulate();

  std::vector<cg::ObjectMeta> object_metas_ = {
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
  };

  cg::Framebuffer fbo_;

  cg::Texture current_frame1_;
  cg::Texture current_frame2_;
  cg::PingPong<cg::Texture> current_frame_;

  cg::Texture last_frame1_;
  cg::Texture last_frame2_;
  cg::PingPong<cg::Texture> last_frame_;

  cg::Camera camera_1_;
};
