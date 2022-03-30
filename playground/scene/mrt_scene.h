#pragma once

#include "engine/camera.h"
#include "engine/color_frame_buffer.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/cube.h"
#include "playground/object/directional_light.h"
#include "playground/object/fullscreen_quad.h"
#include "playground/object/lines.h"
#include "playground/object/plane.h"
#include "playground/object/point_light.h"
#include "playground/scene.h"

class MrtScene : public Scene {
 public:
  void OnEnter(Context* context);
  void OnUpdate(Context* context);
  void OnGui(Context* context);
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

  std::vector<Cube> cubes_;
  std::vector<engine::Transform> cube_transforms_;

  int point_lights_num_ = 100;
  std::vector<PointLight> point_lights_;
  Lines coord_;
  Plane plane_;
  DirectionalLight directional_light_;

  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();

  engine::ColorFrameBuffer mrt_frame_buffer_;
  FullscreenQuad fullscreen_quad_;
};