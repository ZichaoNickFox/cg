#pragma once

#include <unordered_map>

#include "engine/automic_counter.h"
#include "engine/camera.h"
#include "engine/geometry.h"
#include "engine/SSBO.h"
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

  BVH bvh_;

  std::unordered_map<std::string, ModelObject> conell_box_ = {
    {"floor", ModelObject()},
    {"left", ModelObject()},
    {"light", ModelObject()},
    {"right", ModelObject()},
    {"short_box", ModelObject()},
    {"tall_box", ModelObject()},
  };

  engine::Texture canvas_;
  engine::SSBO light_path_ssbo_;
};
