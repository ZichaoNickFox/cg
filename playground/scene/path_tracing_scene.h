#pragma once

#include <unordered_map>

#include "engine/automic_counter.h"
#include "engine/BVH.h"
#include "engine/camera.h"
#include "engine/geometry.h"
#include "engine/primitive.h"
#include "engine/SSBO.h"
#include "playground/object/lines_object.h"
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
  engine::Primitives primitives_;

  struct ObjectData {
    ModelObject object;
    engine::Transform transform;
    glm::vec4 color;
    int mesh_index;
  };

  const int kCornellBoxFloorMeshIndex = 0;
  const int kCornellBoxLeftMeshIndex = 1;
  const int kCornellBoxLightMeshIndex = 2;
  const int kCornellBoxRightMeshIndex = 3;
  const int kCornellBoxShortBoxMeshIndex = 4;
  const int kCornellBoxTallBoxMeshIndex = 5;
  const glm::vec4 kCornellWhite = glm::vec4(0.725f, 0.71f, 0.68f, 1.0f);
  const glm::vec4 kCornellRed = glm::vec4(0.63f, 0.065f, 0.05f, 1.0f);
  const glm::vec4 kCornellGreen = glm::vec4(0.14f, 0.45f, 0.091f, 1.0f);
  const engine::Transform kCornellTransform = {glm::vec3(0, 0, 0), glm::quat(), glm::vec3(0.002, 0.002, 0.002)};
  std::unordered_map<std::string, ObjectData> cornell_box_ = {
    {"cornell_box_floor", {ModelObject(), kCornellTransform, kCornellWhite, kCornellBoxFloorMeshIndex}},
    {"cornell_box_left", {ModelObject(), kCornellTransform, kCornellRed, kCornellBoxLeftMeshIndex}},
    {"cornell_box_light", {ModelObject(), kCornellTransform, kCornellWhite, kCornellBoxLightMeshIndex}},
    {"cornell_box_right", {ModelObject(), kCornellTransform, kCornellGreen, kCornellBoxRightMeshIndex}},
    {"cornell_box_short_box", {ModelObject(), kCornellTransform, kCornellWhite, kCornellBoxShortBoxMeshIndex}},
    {"cornell_box_tall_box", {ModelObject(), kCornellTransform, kCornellWhite, kCornellBoxTallBoxMeshIndex}},
  };

  SphereObject sphere_;

  engine::Texture canvas_;
  engine::SSBO light_path_ssbo_;
  engine::SSBO bvh_ssbo_;

  CoordObject coord_object_;
};
