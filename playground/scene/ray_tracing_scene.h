#pragma once

#include <unordered_map>

#include "engine/camera.h"
#include "engine/geometry.h"
#include "playground/object/sphere_object.h"
#include "playground/scene.h"

class RayTracingScene : public Scene {
 public:
  void OnEnter(Context* context) override;
  void OnUpdate(Context* context) override;
  void OnRender(Context* context) override;
  void OnExit(Context* context) override;

 private:
  void Resterization(Context* context);
  void RayTracing(Context* context);
  void PathTracing(Context* context);

  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();

  std::unordered_map<std::string, engine::Sphere> sphere_map_ = {
    {"light", {{0, 5.4, 1}, {10, 10, 10, 10}, 3}},
    {"ground", {{0, -100.5, 1}, {0.8, 0.8, 0.8, 1.0}, 100}},
    {"ceiling", {{0, 102.5, 1}, {0.8, 0.8, 0.8, 1.0}, 100}},
    {"back_wall", {{0, 1, -101}, {0.8, 0.8, 0.8, 1.0}, 100}},
    {"left_wall", {{-101.5, 0, 1}, {0.0, 0.6, 0.0, 1.0}, 100}},
    {"right_wall", {{101.5, 0, 1}, {0.6, 0.0, 0.0, 1.0}, 100}},
    {"diffuse_ball", {{0, -0.2, 1.5}, {0.8, 0.3, 0.3, 1.0}, 0.3}},
    {"metal_ball", {{0.8, 0.2, 1}, {0.6, 0.8, 0.8, 1.0}, 0.7}},
    {"glass_ball", {{-0.7, 0, 0.5}, {1.0, 1.0, 1.0, 1.0}, 0.5}},
    {"metal2_ball", {{-0.6, -0.3, 2}, {0.8, 0.6, 0.2, 1.0}, 0.2}}
  };
  std::unordered_map<std::string, SphereObject> sphere_object_map_ = {
    {"light", SphereObject()},
    {"ground", SphereObject()},
    {"ceiling", SphereObject()},
    {"back_wall", SphereObject()},
    {"left_wall", SphereObject()},
    {"right_wall", SphereObject()},
    {"diffuse_ball", SphereObject()},
    {"metal_ball", SphereObject()},
    {"glass_ball", SphereObject()},
    {"metal2_ball", SphereObject()}
  };

  engine::Texture canvas_;
};