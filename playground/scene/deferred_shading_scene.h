#pragma once

#include "renderer/camera.h"
#include "renderer/scene.h"
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "playground/context.h"
#include "playground/object/cube_object.h"
#include "playground/object/directional_light_object.h"
#include "playground/object/empty_object.h"
#include "playground/object/lines_object.h"
#include "playground/object/plane_object.h"
#include "playground/object/point_light_object.h"

class DeferredShadingScene : public Scene {
 public:
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  void RenderShadowMap(Scene* context);
  void ForwardShading(Scene* context, const glm::mat4& shadow_map_vp,
                      const renderer::Texture& shadow_map_texture);
  void DeferredShading(Scene* context, const glm::mat4& shadow_map_vp,
                       const renderer::Texture& shadow_map_texture);

  std::string material_name_ = "gold";

  std::vector<CubeObject> cubes_;
  std::vector<renderer::Transform> cube_transforms_;

  int point_lights_num_ = 200;
  std::vector<PointLightObject> point_lights_;
  CoordObject coord_;
  PlaneObject plane_;
  DirectionalLightObject directional_light_;

  renderer::GBuffer gbuffer_;

  std::shared_ptr<renderer::Camera> camera_ = std::make_shared<renderer::Camera>();
 
  EmptyObject deferred_shading_quad_;
};