#include "playground/renderer_scene/disney_scene.h"

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
    ShaderProgramBindings bindings;
    AppendRenderObjectBindings(object, scene.camera(), &bindings);
    DrawBindings(bindings, scene, object);
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
    ShaderProgramBindings bindings;
    AppendCameraBindings(scene.camera(), &bindings);
    AppendResolutionBindings(scene.io().screen_size(), &bindings);
    bindings.SetStorageTexture("texture_position_vs", param.texture_position_vs, TextureAccess::kReadOnly);
    bindings.SetStorageTexture("texture_normal_vs", param.texture_normal_vs, TextureAccess::kReadOnly);
    bindings.SetTexture("texture_depth", param.texture_depth);
    bindings.SetStorageTexture("texture_out", param.texture_out, TextureAccess::kWriteOnly);
    AppendFrameNumBindings(scene.frame_stat().frame_num(), &bindings);
    DispatchBindings(bindings, {
        .workgroup_count = glm::uvec3(scene.io().screen_size().x / 32 + 1,
                                      scene.io().screen_size().y / 32 + 1,
                                      1),
    });
  }
};

void SSAOScene::OnEnter() {
  camera_->mutable_transform()->SetTranslation(glm::vec3(1.78, 0.47, -2.30));
  camera_->mutable_transform()->SetRotation(glm::quat(-0.66, 0.19, -0.70, -0.18));

  Framebuffer::Option option{{3240, 2160}, {kAttachmentPositionVS, kAttachmentNormalVS,
                                            kAttachmentTest, kAttachmentDepth}};
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
  auto geometry_pass = fbo_.BindScoped();
  for (const Object& object : object_repo_.GetObjects()) {
    SSAOGeometryShader(*this, object);
  }
}

void SSAOScene::SSAO() {
  for (const Object& object : object_repo_.GetObjects()) {
    SSAOShader({fbo_.GetTexture("position_vs"), fbo_.GetTexture("normal_vs"),
                fbo_.GetTexture("depth"), fbo_.GetTexture("test")}, *this, object);
  }
}

void SSAOScene::OnExit() {
}
