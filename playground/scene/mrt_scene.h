#pragma once

#include "engine/camera.h"
#include "engine/framebuffer/color_framebuffer.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/cube_object.h"
#include "playground/object/directional_light_object.h"
#include "playground/object/empty_object.h"
#include "playground/object/lines_object.h"
#include "playground/object/plane_object.h"
#include "playground/object/point_light_object.h"
#include "playground/scene.h"

class MrtScene : public Scene {
 public:
  void OnEnter(Context* context);
  void OnUpdate(Context* context);
  void OnRender(Context* contexnt);
  void OnExit(Context* context);

 private:
  void RenderShadowMap(Context* context);
  void RenderScene(Context* context, const glm::mat4& shadow_map_vp,
                   const engine::Texture& shadow_map_texture);

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
  std::vector<engine::Transform> cube_transforms_;

  int point_lights_num_ = 100;
  std::vector<PointLightObject> point_lights_;
  Coord coord_;
  PlaneObject plane_;
  DirectionalLightObject directional_light_;

  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();

  engine::ColorFramebuffer mrt_framebuffer_;
};