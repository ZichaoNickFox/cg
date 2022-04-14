#include "playground/object/skybox.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"

void Skybox::OnUpdate(Context *context) {
}

void Skybox::SetCubeMap(engine::Texture cube_map)
{
  cube_map_ = cube_map;
  has_cube_map_ = true;
}

void Skybox::OnRender(Context *context) {
  if (!material_.HasShader()) {
    material_.PushShader(context->GetShader("skybox"));
  }
  if (context->pass() != pass::kPassDeferredShadingLighting) {
    const engine::Camera& camera = context->camera();
    glm::mat4 project = camera.GetProjectMatrix();
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model = transform_.GetModelMatrix();
    material_.SetMat4("project", project);
    material_.SetMat4("view", view);
    material_.SetMat4("model", model);
  }
  material_.SetTexture("texture0", cube_map_);
  material_.PrepareShader();

  mesh(context)->Submit(); 
}

void Skybox::OnDestory(Context *context) {
}