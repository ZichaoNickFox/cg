#include "playground/object/directional_light.h"

#include <glm/gtx/string_cast.hpp>

#include "playground/util.h"

int DirectionalLight::directional_light_num_ = 0;

void DirectionalLight::Init(Context* context) {
  engine::DepthFrameBuffer::Option option{
      util::Format("directional_light_{}_depth_buffer", directional_light_num_++),
      kShadowMapWidth, kShadowMapHeight};
  depth_frame_buffer_.Init(option);
  billboard_.Init(context, Billboard::Data{"directional_light"});
  shadow_map_camera_->SetType(engine::Camera::Perspective);
}

void DirectionalLight::ShadowMapRenderBegin(Context* context) {
  depth_frame_buffer_.Bind();
  context->PushCamera(shadow_map_camera_);
}

void DirectionalLight::ShadowMapRenderEnd(Context* context) {
  depth_frame_buffer_.Unbind();
  context->PopCamera();
}

glm::mat4 DirectionalLight::GetShadowMapVP() {
  return shadow_map_camera_->GetProjectMatrix() * shadow_map_camera_->GetViewMatrix();
}

void DirectionalLight::OnUpdate(Context* context) {
  billboard_.SetTransform(transform_);
  billboard_.OnUpdate(context);

  glm::vec3 from = transform_.translation();
  glm::vec3 to = transform_.translation() + transform_.rotation() * glm::vec3(0, 0, -10);
  lines_.SetData(context, Lines::Data{{from, to}, {glm::vec3(1, 1, 1), glm::vec3(1, 1, 1)}, GL_LINES, 1});

  shadow_map_camera_->SetTransform(transform_);
}

void DirectionalLight::OnRender(Context* context) {
  billboard_.OnRender(context);
  lines_.OnRender(context);
}