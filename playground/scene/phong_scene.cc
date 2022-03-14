#include "playground/scene/phong_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include <memory>

#include "engine/transform.h"
#include "playground/scene/common.h"

void PhongScene::OnEnter(Context *context)
{
  const glm::vec3 kLightColor = glm::vec3(0.0, 1.0, 0.0);
  const glm::vec3 kLightPos = glm::vec3(1, 1, 0);
  const glm::vec3 kLightScale = glm::vec3(0.4, 0.4, 0.4);
  engine::Transform light_transform(kLightPos, {0, 0, 0}, kLightScale);
  light_.SetTransform(light_transform);
  engine::Material material;
  material.SetShader(context->mutable_shader_repo()->GetOrLoadShader("phong_light"));
  material.SetLocationValue("light_color", kLightColor);
  light_.SetMaterial(material);

  // http://www.barradeau.com/nicoptere/dump/materials.html
  const glm::vec3 kCubePosition = glm::vec3(0, 0, 0);
  engine::Transform cube_transform;
  cube_transform.SetTranslation(kCubePosition);
  engine::Material cube_material;
  cube_material.SetShader(context->mutable_shader_repo()->GetOrLoadShader("phong_cube"));
  cube_material.SetLocationValue("material.ambient", glm::vec3(0.24725, 0.1995, 0.0745));
  cube_material.SetLocationValue("material.diffuse", glm::vec3(0.75164, 0.60648, 0.22648));
  cube_material.SetLocationValue("material.specular", glm::vec3(0.628281, 0.555802, 0.366065));
  cube_material.SetLocationValue("material.shininess", 51.2);
  cube_material.SetLocationValue("light_color", kLightColor);
  cube_material.SetLocationValue("light_pos", kLightPos);
  cube_.SetMaterial(cube_material);

  engine::Camera* camera = context->mutable_camera();
  engine::Transform transform(glm::vec3(glm::vec3(0, 0, 10)), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
  camera->SetTransform(transform);
  camera->SetFront(glm::vec3(0, 0, -1));

  glEnable(GL_DEPTH_TEST);
}

void PhongScene::OnUpdate(Context *context)
{
  MoveCameraByIo(context);
  cube_.OnUpdate(context);
  light_.OnUpdate(context);
}

void PhongScene::OnGui(Context *context)
{
  RenderGoToGallery(context);
  // TODO : Add GUIs (scale, light_color, etc.)
}

void PhongScene::OnRender(Context *context)
{
  cube_.mutable_material()->SetLocationValue("view_pos", context->camera().transform().translation());
  cube_.OnRender(context);
  light_.OnRender(context);
}

void PhongScene::OnExit(Context *context)
{
  cube_.OnDestory(context);
  light_.OnDestory(context);
}