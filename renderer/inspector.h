#pragma once

#include "renderer/texture.h"
#include "renderer/shader.h"

namespace cg {
class Scene;
// Put in OnUpdate before everything
class Inspector {
 public:
  void Inspect(const std::string& scene_name, Scene* scene);

 private:
  void GuiFps(Scene* scene);
  void InspectCamera(Scene* scene);
  void InSpectCursor(Scene* scene);
  void ReloadShaderPrograms(Scene* scene);

  void ShowCoordinators(Scene* scene);

  void InspectObjects(Scene* scene);
  void InspectLights(Scene* scene);
  void InspectMaterials(Scene* scene);
  void InspectNormal(Scene* scene);

  void InspectMesh(Scene* scene, const Object& object);
  void InspectMaterial(int material_index, const std::string& material_name, Material* material);

  bool show_view_coordinator_ = false;
  bool show_world_coordinator_ = false;

  NormalShader::Param normal_shader_param_;
};

class RaytracingDebugCommon {
 public:
  struct LightPath {
    LightPath();
    glm::vec4 light_path[20];
  };
  RaytracingDebugCommon(const cg::Texture& fullscreen_texture, const Scene& scene, const LightPath& light_path);
};
} // namespace cg