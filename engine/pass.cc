#include "engine/pass.h"

namespace engine {
void SceneLightInfo::Insert(const LightInfo& in_light_info) {
  infos.push_back(in_light_info);
}

void SceneLightInfo::Insert(const std::vector<LightInfo>& in_light_infos) {
  infos.insert(infos.end(), in_light_infos.begin(), in_light_infos.end());
}

void DepthBufferPass::Init(Framebuffer* depth_framebuffer,
                           const Transform& camera_transform) {
  camera_->SetType(Camera::Orthographic);
  camera_->SetTransform(camera_transform);
  depth_framebuffer_ = depth_framebuffer;
}

void DepthBufferPass::Begin() {
  depth_framebuffer_->Bind();
}

void DepthBufferPass::End() {
  depth_framebuffer_->Unbind();
}

SceneShadowInfo DepthBufferPass::scene_shadow_info() {
  return SceneShadowInfo::ShadowInfo{camera_vp(), GetTexture(kAttachmentNameDepth)};
}

void ForwardPass::Init(Framebuffer* forward_framebuffer) {
  forward_framebuffer_ = forward_framebuffer;
}

void ForwardPass::Begin() {
  forward_framebuffer_->Bind();
}

void ForwardPass::End() {
  forward_framebuffer_->Unbind();
}

void ShadowPass::Init(const glm::mat4& camera_vp, const Texture& depth_texture,
                      const Texture& color_texture) {
  camera_vp_ = camera_vp;
  depth_texture_ = depth_texture;
  color_texture_ = color_texture;
}

void ShadowPass::Begin() {
}

void ShadowPass::End() {
}
} // namespace engine