#include "playground/renderer_scene/ssao_scene.h"

#include <glm/glm.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "renderer/framebuffer_attachment.h"
#include "renderer/inspector.h"
#include "renderer/transform.h"
#include "base/util.h"

using namespace cg;

class SSAOGeometryShader : public RenderShader {
 public:
  SSAOGeometryShader(const Scene& scene, const Object& object)
      : RenderShader(scene, "ssao_geometry") {
    SetModel(object);
    SetCamera(scene.camera());
    Run(scene, object);
  }
};

class SSAOShader : public ComputeShader {
 public:
  struct Param {
    Texture texture_position_vs;
    Texture texture_normal_vs;
    Texture texture_depth;
    Texture texture_out;
  };
  SSAOShader(const Param& param, const Scene& scene, const Object& object)
      : ComputeShader(scene, "ssao") {
    SetCamera(scene.camera());
    const glm::ivec2 render_size = scene.io().framebuffer_size();
    SetResolution(render_size);
    SetTextureBinding({param.texture_position_vs, "texture_position_vs", TextureAccess::kReadOnly});
    SetTextureBinding({param.texture_normal_vs, "texture_normal_vs", TextureAccess::kReadOnly});
    program_.SetTexture("texture_depth", param.texture_depth);
    SetTextureBinding({param.texture_out, "texture_out", TextureAccess::kWriteOnly});
    SetWorkGroupNum({(render_size.x + 31) / 32, (render_size.y + 31) / 32, 1});
    SetFrameNum(scene);
    Run();
  }
};

void SSAOScene::OnEnter() {
  camera_->mutable_transform()->SetTranslation(glm::vec3(1.78, 0.47, -2.30));
  camera_->mutable_transform()->SetRotation(glm::quat(-0.66, 0.19, -0.70, -0.18));

  Framebuffer::Option option{io_->framebuffer_size(),
                             {kAttachmentPositionVS, kAttachmentNormalVS, kAttachmentTest, kAttachmentDepth}};
  fbo_.Init(option);

  object_repo_.AddOrReplace(object_metas_);
}

void SSAOScene::OnUpdate() {
}

void SSAOScene::OnRender() {
  Geometry();
  SSAO();
  FullscreenQuadShader({fbo_.GetTexture("test")}, *this);
}

void SSAOScene::Geometry() {
  fbo_.Bind();
  for (const Object& object : object_repo_.GetObjects()) {
    SSAOGeometryShader(*this, object);
  }
  fbo_.Unbind();
}

void SSAOScene::SSAO() {
  for (const Object& object : object_repo_.GetObjects()) {
    SSAOShader({fbo_.GetTexture("position_vs"), fbo_.GetTexture("normal_vs"),
                fbo_.GetTexture("depth"), fbo_.GetTexture("test")}, *this, object);
  }
}

void SSAOScene::OnExit() {
}
