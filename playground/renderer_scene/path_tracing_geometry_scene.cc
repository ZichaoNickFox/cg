#include "playground/renderer_scene/path_tracing_geometry_scene.h"

#include <format>

#include "base/math.h"
#include "renderer/filter.h"
#include "renderer/inspector.h"
#include "renderer/object.h"
#include "renderer/shader.h"
#include "renderer/transform.h"

using namespace renderer;

class PathTracingGeometrySceneShader : public renderer::ComputeShader {
 public:
  struct Param {
    std::vector<renderer::Sphere> spheres;
    renderer::Texture canvas;
  };
  PathTracingGeometrySceneShader(const Param& param, const Scene& scene)
      : ComputeShader(scene, "path_tracing_geometry_scene") {
    SetCamera(scene.camera());
    SetTextureBinding({param.canvas, "canvas", GL_READ_ONLY});
    SetResolution(param.canvas.meta().Resolution());
    SetFrameNum(scene);
    for (int i = 0; i < param.spheres.size(); ++i) {
      const Sphere& sphere = param.spheres[i];
      program_.SetInt(std::format("spheres[{}].id", i), sphere.id);
      program_.SetVec3(std::format("spheres[{}].center_pos", i), sphere.translation);
      program_.SetVec4(std::format("spheres[{}].color", i), sphere.color);
      program_.SetFloat(std::format("spheres[{}].radius", i), sphere.radius);
    }
    Run();
  }
};

void PathTracingGeometryScene::OnEnter() {
  camera_->SetPerspectiveFov(60);
  camera_->SetTransform({{0, 1, 5}, glm::quat(1, -0.02, 0, 0), {1, 1, 1}});

  // path tracing
  glm::ivec2 viewport_size = io_->screen_size();
  std::vector<glm::vec4> canvas(viewport_size.x * viewport_size.y, kBlack);
  canvas_ = CreateTexture2D(viewport_size.x, viewport_size.y, canvas, GL_NEAREST, GL_NEAREST);

  glEnable_(GL_DEPTH_TEST);
}

void PathTracingGeometryScene::OnUpdate() {
}

void PathTracingGeometryScene::OnRender() {
  PathTracingGeometrySceneShader({util::AsValueVector(sphere_map_), canvas_}, *this);
  FullscreenQuadShader({canvas_}, *this);
}

void PathTracingGeometryScene::OnExit() {
}