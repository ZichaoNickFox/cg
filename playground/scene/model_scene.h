#pragma once

#include "engine/camera.h"
#include "engine/shader.h"
#include "engine/texture.h"
#include "engine/transform.h"
#include "playground/context.h"
#include "playground/object/cube_object.h"
#include "playground/object/lines_object.h"
#include "playground/object/model_object.h"
#include "playground/object/point_light_object.h"
#include "playground/scene.h"

class ModelScene : public Scene {
 public:
  void OnEnter(Context* context);
  void OnUpdate(Context* context);
  void OnRender(Context* contexnt);
  void OnExit(Context* context);

 private:
  float shininess_ = 1;
  bool use_texture_ambient_ = true;
  bool use_texture_normal_ = true;
  bool use_texture_specular_ = true;
  bool use_texture_diffuse_ = true;
  bool use_texture_height_ = true;
  bool show_vertex_normal_ = false;
  bool show_texture_normal_ = false;
  bool show_TBN_ = false;
  bool show_triangle_ = false;
  bool use_blinn_phong_ = false;

  float rotate_speed_ = 0.01;

  std::shared_ptr<engine::Camera> camera_ = std::make_shared<engine::Camera>();
  ModelObject nanosuit_;
  std::vector<PointLightObject> point_lights_;

  bool enable_cull_face_ = false;
  int call_face_ = GL_BACK;
  int cw_ = GL_CW;

  CoordObject coord_;
};