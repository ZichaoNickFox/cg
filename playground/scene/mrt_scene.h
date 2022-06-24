#pragma once

#include "renderer/camera.h"
#include "renderer/framebuffer/color_framebuffer.h"
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

class MrtScene : public Scene {
 public:
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  void RenderShadowMap(Scene* context);
  void RenderScene(Scene* context, const glm::mat4& shadow_map_vp,
                   const renderer::Texture& shadow_map_texture);

  struct MaterialProperty {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
  };
  MaterialProperty gold_ = {
    glm::vec4(0.24725, 0.1995, 0.0745, 1),
    glm::vec4(0.75164, 0.60648, 0.22648, 1),
    glm::vec4(0.628281, 0.555802, 0.366065, 1),
    51.2};
  MaterialProperty material_property_ = gold_;

  std::vector<CubeObject> cubes_;
  std::vector<renderer::Transform> cube_transforms_;

  int point_lights_num_ = 100;
  std::vector<PointLightObject> point_lights_;
  CoordObject coord_;
  PlaneObject plane_;
  DirectionalLightObject directional_light_;

  std::shared_ptr<renderer::Camera> camera_ = std::make_shared<renderer::Camera>();

  renderer::ColorFramebuffer mrt_framebuffer_;
};