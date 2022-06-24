#include "playground/scene/geometry_scene.h"

#include <glm/glm.hpp>

#include "renderer/color.h"
#include "renderer/math.h"
#include "playground/scene/common.h"
#include "playground/shaders.h"

void GeometryScene::OnEnter(Scene* context) {
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

  context->SetCamera(camera_.get());
}

void GeometryScene::OnUpdate() {
  OnUpdateCommon _(scene, "RayTriangleScene");
}

void GeometryScene::OnRender() {
  renderer::Ray ray = scene->camera().GetPickRay(scene->io().GetCursorPosSS());
  for (int i = 0; i < kTriangleNum; ++i) {
    glm::vec4 color = renderer::kBlack;
    if (RayTriangle(ray, triangles_[i]).hitted) {
      color = renderer::kRed;
    }

    LinesObject lines_object;
    LinesObject::Mesh mesh(triangles_[i], color);
    lines_object.SetMesh(mesh);
    LinesShader lines_shader({}, scene, &lines_object);
    lines_object.OnRender(scene);
  }

  for (int i = 0; i < kAABBNum; ++i) {
    glm::vec4 color = renderer::kBlack;
    if (RayAABB(ray, aabbs_[i]).hitted) {
      color = renderer::kRed;
    }

    LinesObject lines_object;
    LinesObject::Mesh mesh({aabbs_[i]}, color);
    lines_object.SetMesh(mesh);
    LinesShader lines_shader({}, scene, &lines_object);
    lines_object.OnRender(scene);
  }

  OnRenderCommon _(scene);
}

void GeometryScene::OnExit(Scene* context) {
}
