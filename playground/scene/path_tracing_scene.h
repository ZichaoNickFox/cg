#pragma once

#include "engine/camera.h"
#include "playground/object/sphere.h"
#include "playground/scene.h"

class PathTracingScene : public Scene {
 public:
  void OnEnter(Context* context) override;
  void OnUpdate(Context* context) override;
  void OnRender(Context* context) override;
  void OnExit(Context* context) override;

 private:
  void Resterization(Context* context);
  void RayTracing(Context* context);

  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();
  Sphere light_;
  Sphere ground_;
  Sphere ceiling_;
  Sphere back_wall_;
  Sphere right_wall_;
  Sphere left_wall_;

  Sphere diffuse_ball_;
  Sphere metal_ball_;
  Sphere glass_ball_;
  Sphere metal_ball2_;

  std::vector<glm::vec3> canvas_;
  engine::Texture texture_canvas_;
};
