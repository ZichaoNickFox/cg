#pragma once

#include "engine/camera.h"
#include "engine/framebuffer/depth_framebuffer.h"
#include "engine/framebuffer/gbuffer.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/cube.h"
#include "playground/object/directional_light.h"
#include "playground/object/empty_object.h"
#include "playground/object/lines.h"
#include "playground/object/plane.h"
#include "playground/object/point_light.h"
#include "playground/scene.h"

class DeferredShadingScene : public Scene {
 public:
  void OnEnter(Context* context);
  void OnUpdate(Context* context);
  void OnRender(Context* contexnt);
  void OnExit(Context* context);

 private:
  void RenderShadowMap(Context* context);
  void ForwardShading(Context* context, const glm::mat4& shadow_map_vp,
                      const engine::Texture& shadow_map_texture);
  void DeferredShading(Context* context, const glm::mat4& shadow_map_vp,
                       const engine::Texture& shadow_map_texture);

  std::string material_name_ = "gold";

  std::vector<Cube> cubes_;
  std::vector<engine::Transform> cube_transforms_;

  int point_lights_num_ = 200;
  std::vector<PointLight> point_lights_;
  Coord coord_;
  Plane plane_;
  DirectionalLight directional_light_;

  engine::GBuffer gbuffer_;

  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();
 
  EmptyObject deferred_shading_quad_;
};