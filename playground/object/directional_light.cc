#include "playground/object/directional_light.h"

void DirectionalLight::Init() {
  depth_frame_buffer_.Init(shadow_map_width, shadow_map_height);
}

void DirectionalLight::ShadowMapRenderBegin(Context* context) {
  depth_frame_buffer_.Bind();
  context->PushCamera(shadow_map_camera_);
}

void DirectionalLight::ShadowMapRenderEnd(Context* context) {
  depth_frame_buffer_.Unbind();
  context->PopCamera();
}