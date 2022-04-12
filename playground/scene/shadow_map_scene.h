#pragma once

#include "engine/camera.h"
#include "engine/depth_frame_buffer.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "playground/context.h"
#include "playground/object/coord.h"
#include "playground/object/cube.h"
#include "playground/object/directional_light.h"
#include "playground/object/lines.h"
#include "playground/object/plane.h"
#include "playground/object/point_light.h"
#include "playground/scene.h"

class ShadowMapScene : public Scene {
 public:
  void OnEnter(Context* context);
  void OnUpdate(Context* context);
  void OnRender(Context* contexnt);
  void OnExit(Context* context);

 private:
  void RenderShadowMap(Context* context);
  void RenderScene(Context* context, const glm::mat4& shadow_map_vp,
                   const engine::Texture& shadow_map_texture);

  glm::vec3 light_pos_ = glm::vec3(1, 1, 0);
  glm::vec3 light_scale_ = glm::vec3(.2, .2, .2);
  glm::vec3 light_color_ = glm::vec3(1, 1, 1);

  std::vector<Cube> cubes_;
  std::vector<engine::Transform> cube_transforms_;

  PointLight light_;
  Coord coord_;
  Plane plane_;
  DirectionalLight directional_light_;

  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();
};
#pragma once

