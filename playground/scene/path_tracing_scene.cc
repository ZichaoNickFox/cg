#include "playground/scene/path_tracing_scene.h"

#include "engine/transform.h"
#include "engine/math.h"
#include "playground/scene/common.h"
#include "playground/shaders.h"

void PathTracingScene::OnEnter(Context* context) {
  for (auto& pair : sphere_data_map_) {
    const std::string& name = pair.first;
    const engine::SphereData& sphere_data = pair.second;
    const glm::vec3& translation = sphere_data.translation;
    const float& r = sphere_data.radius;
    sphere_map_[name].mutable_transform()->SetTranslation(translation);
    sphere_map_[name].mutable_transform()->SetScale(glm::vec3(r, r, r));
  }

  camera_->SetPerspectiveFov(60);
  camera_->SetTransform({{0, 1, 5}, glm::quat(1, -0.02, 0, 0), {1, 1, 1}});
  context->SetCamera(camera_.get());

  // path tracing
  glm::ivec2 viewport_size = context->io().screen_size();
  canvas_.resize(viewport_size.x * viewport_size.y);
  for (glm::vec3& elem : canvas_) {
    elem = glm::vec3(0,0,0);
  }
  texture_canvas_ = context->CreateTexture({viewport_size.x, viewport_size.y, canvas_, GL_NEAREST, GL_NEAREST});

  glEnable_(GL_DEPTH_TEST);
}

void PathTracingScene::OnUpdate(Context* context) {
  OnUpdateCommon _(context, "PathTracing");
}

void PathTracingScene::OnRender(Context* context) {
  //Resterization(context);
  ColorOnly(context);
  //RayTracing(context);
}

void PathTracingScene::OnExit(Context* context) {
}

void PathTracingScene::Resterization(Context* context) {
  for (auto& pair : sphere_data_map_) {
    const std::string& name = pair.first;
    const engine::SphereData& sphere_data = pair.second;
    const glm::vec4& color = sphere_data.color;
    ColorShader({color}, context, &sphere_map_[name]);
    sphere_map_[name].OnRender(context);
  }

  OnRenderCommon _(context);
}

void PathTracingScene::ColorOnly(Context* context) {
  ColorOnlyShader();
}

void PathTracingScene::RayTracing(Context* context) {
/*
  engine::PathTracing path_tracing({});
  glm::vec2 viewport_size = context->io().screen_size();
  int ray_num = viewport_size.x  * viewport_size.y;
  for (int i = 0; i < ray_num; ++i) {
    float u = engine::RandFromTo(0, viewport_size.x - 1) / viewport_size.x;
    float v = engine::RandFromTo(0, viewport_size.y - 1) / viewport_size.y;
    glm::vec3 near, far;
    engine::Ray ray(near, glm::normalize(far - near));
    glm::vec3 color = path_tracing.GetRayColor(ray);
    canvas_[u * viewport_size.x][v * viewport_size.y] = color;
  }
  */

  glm::vec2 viewport_size = context->io().screen_size();
  for (int i = 0; i < viewport_size.x * viewport_size.y; ++i) {
    //canvas_[i] = glm::vec3(engine::RandFromTo(0,1), engine::RandFromTo(0,1), engine::RandFromTo(0,1));
  }
  glBindTexture_(GL_TEXTURE_2D, texture_canvas_.id());
  glTexSubImage2D_(GL_TEXTURE_2D, 0, 0, 0, viewport_size.x, viewport_size.y,
                   GL_RGB, GL_FLOAT, canvas_.data());
  
  EmptyObject object;
  FullscreenQuadShader({texture_canvas_}, context, &object);
  object.OnRender(context);
}
