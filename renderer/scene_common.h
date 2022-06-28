#pragma once

#include "renderer/scene.h"
#include "renderer/texture.h"

namespace renderer {
// Put in OnUpdate before everything
class OnUpdateCommon {
 public:
  OnUpdateCommon(Scene* scene, const std::string& title);
  ~OnUpdateCommon();

 private:
  void GuiFps(Scene* scene);
  void InspectCamera(Scene* scene);
  void MoveCamera(Scene* scene);
  void InSpectCursor(Scene* scene);
  void ReloadShaderPrograms(Scene* scene);
  void InSpectObjects(Scene* scene);
  void InspectLights(Scene* scene);

  void InspectMesh(Scene* scene, const Object& object);
  void InspcetMaterial(Scene* scene, const Object& object);
};

// Put in OnRender after everything
class OnRenderCommon {
 public:
  OnRenderCommon(Scene* scene);

 private:
  void DrawViewCoord(Scene* scene);
};

class RaytracingDebugCommon {
 public:
  struct LightPath {
    LightPath();
    glm::vec4 light_path[20];
  };
  RaytracingDebugCommon(const renderer::Texture& fullscreen_texture, const Scene& scene, const LightPath& light_path);
};
} // namespace renderer