#pragma once

#include "renderer/camera.h"
#include "renderer/material.h"
#include "renderer/object.h"
#include "renderer/scene.h"

namespace renderer {

class RenderShader {
 protected:
  RenderShader(const Scene& scene, const std::string& shader_name);
  void Run(const Scene& scene, const Object& object) const;

 public:
  void SetModel(const glm::mat4& model);
  void SetModel(const Object& object);
  void SetCamera(const Camera& camera);
  void SetMaterialIndex(int material_index);
  void SetNearFar(const Camera& camera);

 protected:
  Shader shader_;
};

} // namespace renderer