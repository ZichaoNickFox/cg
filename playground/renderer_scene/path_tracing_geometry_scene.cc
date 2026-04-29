#include "playground/renderer_scene/path_tracing_geometry_scene.h"

#include "base/math.h"
#include "base/util.h"
#include "renderer/filter.h"
#include "renderer/inspector.h"
#include "renderer/object.h"
#include "renderer/shader.h"
#include "renderer/transform.h"

using namespace cg;

class PathTracingGeometrySceneShader : public cg::ComputeShader {
 public:
  struct Param {
    std::vector<cg::Sphere> spheres;
    cg::Texture canvas;
  };
  PathTracingGeometrySceneShader(const Param& param, const Scene& scene)
      : ComputeShader(scene, "path_tracing_geometry_scene") {
    ShaderProgramBindings bindings;
    AppendCameraBindings(scene.camera(), &bindings);
    bindings.SetStorageTexture("canvas", param.canvas, TextureAccess::kWriteOnly);
    AppendResolutionBindings(param.canvas.meta().Resolution(), &bindings);
    AppendFrameNumBindings(scene.frame_stat().frame_num(), &bindings);
    for (int i = 0; i < param.spheres.size(); ++i) {
      const Sphere& sphere = param.spheres[i];
      bindings.SetInt(util::Format("spheres[{}].id", i), sphere.id);
      bindings.SetVec3(util::Format("spheres[{}].center_pos", i), sphere.translation);
      bindings.SetVec4(util::Format("spheres[{}].color", i), sphere.color);
      bindings.SetFloat(util::Format("spheres[{}].radius", i), sphere.radius);
    }
    DispatchBindings(bindings, {
        .workgroup_count = glm::uvec3((param.canvas.meta().width + 31) / 32,
                                      (param.canvas.meta().height + 31) / 32,
                                      1),
    });
  }
};

void PathTracingGeometryScene::OnEnter() {
  camera_->SetPerspectiveFov(60);
  camera_->SetTransform({{0, 1, 5}, glm::quat(1, -0.02, 0, 0), {1, 1, 1}});

  // path tracing
  glm::ivec2 viewport_size = io_->framebuffer_size();
  std::vector<glm::vec4> canvas(viewport_size.x * viewport_size.y, kBlack);
  canvas_ = CreateTexture2D(viewport_size.x, viewport_size.y, canvas,
                            rhi::FilterMode::kNearest, rhi::FilterMode::kNearest);
}

void PathTracingGeometryScene::OnUpdate() {
}

void PathTracingGeometryScene::OnRender() {
  PathTracingGeometrySceneShader({util::AsValueVector(sphere_map_), canvas_}, *this);
  FullscreenQuadShader({canvas_}, *this);
}

void PathTracingGeometryScene::OnExit() {
}
