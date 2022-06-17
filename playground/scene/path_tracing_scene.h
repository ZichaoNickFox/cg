#pragma once

#include <unordered_map>

#include "engine/automic_counter.h"
#include "engine/BVH.h"
#include "engine/camera.h"
#include "engine/geometry.h"
#include "engine/SSBO.h"
#include "playground/object/model_object.h"
#include "playground/object/sphere_object.h"
#include "playground/scene.h"

class PathTracingScene : public Scene {
 public:
  void OnEnter(Context* context) override;
  void OnUpdate(Context* context) override;
  void OnRender(Context* context) override;
  void OnExit(Context* context) override;

 private:
  void Rasterization(Context* context);
  void PathTracing(Context* context);

  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();

  engine::BVH bvh_;

  struct ObjectData {
    ModelObject object;
    glm::vec4 color;
    int primitive_index;
  };

  const int kCornellBoxFloorPrimitiveIndex = 0;
  const int kCornellBoxLeftPrimitiveIndex = 1;
  const int kCornellBoxLightPrimitiveIndex = 2;
  const int kCornellBoxRightPrimitiveIndex = 3;
  const int kCornellBoxShortBoxPrimitiveIndex = 4;
  const int kCornellBoxTallBoxPrimitiveIndex = 5;
  std::unordered_map<std::string, ObjectData> conell_box_ = {
    {"cornell_box_floor", {ModelObject(), glm::vec4(0.725f, 0.71f, 0.68f, 1.0f), kCornellBoxFloorPrimitiveIndex}},
    {"cornell_box_left", {ModelObject(), glm::vec4(0.14f, 0.45f, 0.091f, 1.0f), kCornellBoxLeftPrimitiveIndex}},
    {"cornell_box_light", {ModelObject(), glm::vec4(0.725f, 0.71f, 0.68f, 1.0f), kCornellBoxLightPrimitiveIndex}},
    {"cornell_box_right", {ModelObject(), glm::vec4(0.63f, 0.065f, 0.05f, 1.0f), kCornellBoxRightPrimitiveIndex}},
    {"cornell_box_short_box", {ModelObject(), glm::vec4(0.725f, 0.71f, 0.68f, 1.0f), kCornellBoxShortBoxPrimitiveIndex}},
    {"cornell_box_tall_box", {ModelObject(), glm::vec4(0.725f, 0.71f, 0.68f, 1.0f), kCornellBoxTallBoxPrimitiveIndex}},
  };

  SphereObject sphere_;

  engine::Texture canvas_;
  engine::SSBO light_path_ssbo_;
};
