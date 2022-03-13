#include "playground/scene/phong_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include <memory>

#include "playground/scene/common.h"

void PhongScene::OnEnter(Context *context)
{
  Cube::Option cube_option;
  cube_option.position = glm::vec3(0, 0, 0);
  cube_option.material.SetShader(
    context->shader_repo().GetShader("phong_cube"));
  cube_.Init(cube_option, context);

  Cube::Option point_light_option;
  point_light_option.position = glm::vec3(2, 2, 0);
  point_light_option.scale = glm::vec3(0.2, 0.2, 0.2);
  point_light_option.material.SetShader(
    context->shader_repo().GetShader("phong_light"));
  point_light_.Init(point_light_option, context);

  engine::Camera* camera = context->mutable_camera();
  camera->SetPosition(glm::vec3(0, 0, 10));
  camera->SetFront(glm::vec3(0, 0, -1));

  glEnable(GL_DEPTH_TEST);
}

void PhongScene::OnUpdate(Context *context)
{
  MoveCameraByIo(context);
  cube_.OnUpdate(context);
  point_light_.OnUpdate(context);
}

void PhongScene::OnGui(Context *context)
{
  RenderGoToGallery(context);
}

void PhongScene::OnRender(Context *context)
{
  cube_.OnRender(context);
  point_light_.OnRender(context);
}

void PhongScene::OnExit(Context *context)
{
  cube_.OnDestory(context);
  point_light_.OnDestory(context);
}