#include "playground/scene/geometry_scene.h"

#include <glm/glm.hpp>

#include "engine/color.h"
#include "engine/math.h"
#include "playground/scene/common.h"
#include "playground/shaders.h"

void GeometryScene::OnEnter(Context* context) {
  for (int i = 0; i < kTriangleNum; ++i) {
    glm::vec3 center = glm::vec3(util::RandFromTo(-5, 5), util::RandFromTo(-5, 5), util::RandFromTo(-5, 5));
    glm::vec3 a_offset = glm::vec3(util::RandFromTo(-0.5, 0.5), util::RandFromTo(-0.5, 0.5),
                                   util::RandFromTo(-0.5, 0.5));
    glm::vec3 b_offset = glm::vec3(util::RandFromTo(-0.5, 0.5), util::RandFromTo(-0.5, 0.5),
                                   util::RandFromTo(-0.5, 0.5));
    glm::vec3 c_offset = glm::vec3(util::RandFromTo(-0.5, 0.5), util::RandFromTo(-0.5, 0.5),
                                   util::RandFromTo(-0.5, 0.5));
    triangles_[i] = engine::Triangle{center + a_offset, center + b_offset, center + c_offset};
  }

  for (int i = 0; i < kAABBNum; ++i) {
    glm::vec3 center = glm::vec3(util::RandFromTo(-5, 5), util::RandFromTo(-5, 5), util::RandFromTo(-5, 5));
    glm::vec3 offset = glm::vec3(util::RandFromTo(0.1, 0.5), util::RandFromTo(0.1, 0.5), util::RandFromTo(0.1, 0.5));
    aabbs_[i] = engine::AABB{center + offset, center - offset};
  }

  context->SetCamera(camera_.get());
}

void GeometryScene::OnUpdate(Context* context) {
  OnUpdateCommon _(context, "RayTriangleScene");
}

void GeometryScene::OnRender(Context* context) {
  engine::Ray ray = context->camera().GetPickRay(context->io().GetCursorPosSS());
  for (int i = 0; i < kTriangleNum; ++i) {
    glm::vec4 color = engine::kBlack;
    if (RayTriangle(ray, triangles_[i]).hitted) {
      color = engine::kRed;
    }

    LinesObject lines_object;
    LinesObject::Mesh mesh(triangles_[i], color);
    lines_object.SetMesh(mesh);
    LinesShader lines_shader({}, context, &lines_object);
    lines_object.OnRender(context);
  }

  for (int i = 0; i < kAABBNum; ++i) {
    glm::vec4 color = engine::kBlack;
    if (RayAABB(ray, aabbs_[i]).hitted) {
      color = engine::kRed;
    }

    LinesObject lines_object;
    LinesObject::Mesh mesh({aabbs_[i]}, color);
    lines_object.SetMesh(mesh);
    LinesShader lines_shader({}, context, &lines_object);
    lines_object.OnRender(context);
  }

  OnRenderCommon _(context);
}

void GeometryScene::OnExit(Context* context) {
}
