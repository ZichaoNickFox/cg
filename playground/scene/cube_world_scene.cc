#include "playground/scene/cube_world_scene.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include <memory>

#include "playground/scene/common.h"
#include "playground/util.h"

void CubeWorldScene::OnEnter(Context *context)
{
  cube_positions_[0] = glm::vec3(0.0f, 0.0f, 0.0f);
  cube_positions_[1] = glm::vec3(2.0f, 5.0f, -15.0f);
  cube_positions_[2] = glm::vec3(-1.5f, -2.2f, -2.5f);
  cube_positions_[3] = glm::vec3(-3.8f, -2.0f, -12.3f);
  cube_positions_[4] = glm::vec3(2.4f, -0.4f, -3.5f);
  cube_positions_[5] = glm::vec3(-1.7f, 3.0f, -7.5f);
  cube_positions_[6] = glm::vec3(1.3f, -2.0f, -2.5f);
  cube_positions_[7] = glm::vec3(1.5f, 2.0f, -2.5f);
  cube_positions_[8] = glm::vec3(1.5f, 0.2f, -1.5f);
  cube_positions_[9] = glm::vec3(-1.3f, 1.0f, -1.5f);

  for (int i = 0; i < kCubeNum; ++i) {
    std::unique_ptr<Cube> cube = std::make_unique<Cube>();
    engine::Transform cube_transform(cube_positions_[i], glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
    cube->SetTransform(cube_transform);
    engine::Material material;
    material.SetTexture("texture0", context->mutable_texture_repo()->GetOrLoadTexture("cube_texture"));
    material.PushShader(context->mutable_shader_repo()->GetOrLoadShader("cube"));
    cube->SetMaterial(material);
    cubes_.push_back(std::move(cube));
  }

  glEnable(GL_DEPTH_TEST);

  engine::Transform camera_transform(glm::vec3(0, 0, 10), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
  camera_->SetTransform(camera_transform);
  // camera_->SetFront(glm::vec3(0, 0, -1));
  context->PushCamera(camera_);
}

void CubeWorldScene::OnUpdate(Context *context)
{
  ControlCameraByIo(context);
}

void CubeWorldScene::OnGui(Context *context)
{
  RenderGoToGallery(context);
}

void CubeWorldScene::OnRender(Context *context)
{
  for (const std::unique_ptr<Cube>& cube : cubes_) {
    cube->OnRender(context);
  }
}

void CubeWorldScene::OnExit(Context *context)
{
  context->PopCamera();
}