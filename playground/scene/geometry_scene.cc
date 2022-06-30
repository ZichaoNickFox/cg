#include "playground/scene/geometry_scene.h"

#include <glm/glm.hpp>

#include "renderer/color.h"
#include "renderer/inspector.h"
#include "renderer/math.h"
#include "renderer/shader.h"

using namespace renderer;

class GeometrySceneShader : public ComputeShader {
 public:
  struct Param {
    renderer::Texture canvas;
    size_t triangle_num;
    size_t aabb_num;
  };
  GeometrySceneShader(const Param& param, const Scene& scene)
      : ComputeShader(scene, "geometry_scene") {
    SetCamera(scene.camera());
    SetTextureBinding({param.canvas, "canvas", GL_READ_WRITE, GL_RGBA32F});
    SetScreenSize(scene.io().screen_size());
    program_.SetInt("triangle_num", param.triangle_num);
    program_.SetInt("aabb_num", param.aabb_num);
    Run();
  }
};

void GeometryScene::OnEnter() {
  for (int i = 0; i < kTriangleNum; ++i) {
    glm::vec3 center = glm::vec3(util::RandFromTo(-5, 5), util::RandFromTo(-5, 5), util::RandFromTo(-5, 5));
    glm::vec3 a_offset = glm::vec3(util::RandFromTo(-0.5, 0.5), util::RandFromTo(-0.5, 0.5),
                                   util::RandFromTo(-0.5, 0.5));
    glm::vec3 b_offset = glm::vec3(util::RandFromTo(-0.5, 0.5), util::RandFromTo(-0.5, 0.5),
                                   util::RandFromTo(-0.5, 0.5));
    glm::vec3 c_offset = glm::vec3(util::RandFromTo(-0.5, 0.5), util::RandFromTo(-0.5, 0.5),
                                   util::RandFromTo(-0.5, 0.5));
    triangles_[i] = renderer::Triangle{center + a_offset, center + b_offset, center + c_offset};
  }

  for (int i = 0; i < kAABBNum; ++i) {
    glm::vec3 center = glm::vec3(util::RandFromTo(-5, 5), util::RandFromTo(-5, 5), util::RandFromTo(-5, 5));
    glm::vec3 offset = glm::vec3(util::RandFromTo(0.1, 0.5), util::RandFromTo(0.1, 0.5), util::RandFromTo(0.1, 0.5));
    aabbs_[i] = renderer::AABB{center + offset, center - offset};
  }

  std::vector<renderer::TriangleGPU> triangle_gpus; 
  std::vector<renderer::AABBGPU> aabb_gpus;
  std::transform(triangles_.begin(), triangles_.end(), std::back_inserter(triangle_gpus),
                 [] (const renderer::Triangle& triangle) { return renderer::TriangleGPU(triangle, 0); });
  std::transform(aabbs_.begin(), aabbs_.end(), std::back_inserter(aabb_gpus),
                 [] (const renderer::AABB& aabb) { return renderer::AABBGPU(aabb); });
  ssbo_triangle_.SetData(util::VectorSizeInByte(triangle_gpus), triangle_gpus.data());
  ssbo_aabb_.SetData(util::VectorSizeInByte(aabb_gpus), aabb_gpus.data());

  glm::ivec2 viewport_size = io_->screen_size();
  std::vector<glm::vec4> canvas(viewport_size.x * viewport_size.y, kBlack);
  canvas_ = CreateTexture2D(viewport_size.x, viewport_size.y, canvas, GL_NEAREST, GL_NEAREST);
}

void GeometryScene::OnUpdate() {
}

void GeometryScene::Rasterization() {
  renderer::Ray ray = camera_->GetPickRay(io_->GetCursorPosSS());
  for (int i = 0; i < kTriangleNum; ++i) {
    glm::vec4 color = renderer::kBlack;
    if (RayTriangle(ray, triangles_[i]).hitted) {
      color = renderer::kRed;
    }

    LinesShader({}, *this, {{triangles_[i]}, color});
  }

  for (int i = 0; i < kAABBNum; ++i) {
    glm::vec4 color = renderer::kBlack;
    if (RayAABB(ray, aabbs_[i]).hitted) {
      color = renderer::kRed;
    }

    LinesShader({}, *this, {{aabbs_[i]}, color});
  }
}

void GeometryScene::Raytracing() {
  GeometrySceneShader({canvas_, triangles_.size(), aabbs_.size()}, *this);
  FullscreenQuadShader({canvas_}, *this);
}

void GeometryScene::OnRender() {
  // Rasterization();
  Raytracing();
}

void GeometryScene::OnExit() {
}
