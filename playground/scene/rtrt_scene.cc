#include "playground/scene/rtrt_scene.h"

#include "glm/gtx/string_cast.hpp"
#include <memory>

#include "renderer/color.h"
#include "renderer/inspector.h"
#include "renderer/math.h"
#include "renderer/mesh/lines_mesh.h"
#include "renderer/shader.h"
#include "renderer/transform.h"

using namespace renderer;

class RTRTGeometryShader : public renderer::RenderShader {
 public:
  struct Param {
    glm::vec4 color = glm::vec4(1, 0, 0, 1);
  };
  RTRTGeometryShader(const Param& param, const Scene& scene, const Object& object)
      : RenderShader(scene, "rtrt_geometry") {
    const Camera& camera = scene.camera();
    SetModel(object);
    SetCamera(camera);
    program_.SetVec4("color", param.color);
    Run(scene, object);
  }
};

class RTRTPathTracingShader : public renderer::ComputeShader {
 public:
  struct Param {
    glm::vec2 resolution;
    renderer::Texture rasterized_color;
    renderer::Texture rasterized_position_ws;
    renderer::Texture rasterized_surface_normal_ws;
  };
  RTRTPathTracingShader(const Param& param, const Scene& scene) 
      : ComputeShader(scene, "rtrt_path_tracing") {
    SetCamera(scene.camera());

    SetTextureBinding({param.rasterized_color, "in_rasterized_color", GL_READ_WRITE});
    SetTextureBinding({param.rasterized_position_ws, "in_rasterized_position_ws", GL_READ_ONLY});
    SetTextureBinding({param.rasterized_surface_normal_ws, "in_rasterized_surface_normal_ws", GL_READ_ONLY});

    SetFrameNum(scene.frame_stat().frame_num());
    SetWorkGroupNum({param.resolution.x / 32 + 1, param.resolution.y / 32 + 1, 1});
    SetResolution(param.resolution);
    Run();
  }
};

class RTRTOutlierClampingShader : public renderer::ComputeShader {
 public:
  struct Param {
    glm::vec2 resolution;
    renderer::Texture ping_color;
    renderer::Texture pong_color;
  };
  RTRTOutlierClampingShader(const Param& param, const Scene& scene) 
      : ComputeShader(scene, "rtrt_outlier_clamping") {
    SetTextureBinding({param.ping_color, "texture_in", GL_READ_ONLY});
    SetTextureBinding({param.pong_color, "texture_out", GL_WRITE_ONLY});
    SetWorkGroupNum({param.resolution.x / 32 + 1, param.resolution.y / 32 + 1, 1});
    SetResolution(param.resolution);
    Run();
  }
};

class RTRTDenoiseShader : public renderer::ComputeShader {
 public:
  struct Param {
    glm::vec2 resolution;
    renderer::Texture texture;
  };
  RTRTDenoiseShader(const Param& param, const Scene& scene) 
      : ComputeShader(scene, "rtrt_denoise") {
    SetTextureBinding({param.texture, "texture", GL_READ_WRITE});
    SetWorkGroupNum({param.resolution.x / 32 + 1, param.resolution.y / 32 + 1, 1});
    Run();
  }
};

void RTRTScene::OnEnter() {
  camera_->SetPerspectiveFov(60);
  camera_->SetTransform({{glm::vec3(0.286482, 0.708117, -1.065640)},
                          glm::quat(0.070520, {-0.005535, -0.994436, -0.078057}), {1, 1, 1}});

  // path tracing
  glm::vec3 canvas_size(3240, 2160, 0);
  std::vector<renderer::FramebufferAttachment> fbo1_attachments = {
    kAttachmentColor, kAttachmentPositionWS, kAttachmentSurfaceNormalWS, kAttachmentDepth
  };
  std::vector<renderer::FramebufferAttachment> fbo2_attachments = { kAttachmentColor };
  fbo1_.Init({canvas_size, fbo1_attachments, kBlack});
  fbo2_.Init({canvas_size, fbo2_attachments, kBlack});

  object_repo_.AddOrReplace(object_metas_);
  object_repo_.GetPrimitives(mesh_repo_, material_repo_, {}, &primitive_repo_);
  bvh_.Build(primitive_repo_, {100, BVH::Partition::kPos, 64});

  ping_pong_.Init(&fbo1_, &fbo2_);

  glEnable_(GL_DEPTH_TEST);
}

void RTRTScene::OnUpdate() {
}

void RTRTScene::OnRender() {
  ping_pong_.SetPing(&fbo1_);
  Rasterization();
  PathTracing();
  OutlierClamping();
  // Denoise();
  // TemproalAccumulate();
  FullscreenQuadShader({ping_pong_.ping()->GetTexture("color")}, *this);
}

void RTRTScene::OnExit() {
}

void RTRTScene::Rasterization() {
  ping_pong_.ping()->Bind();
  for (const Object& object : object_repo_.GetObjects({Filter::kExcludes, {"sphere"}})) {
    RTRTGeometryShader({material_repo_.GetMaterial(object.material_index).diffuse}, *this, object);
  }
  ping_pong_.ping()->Unbind();
}

void RTRTScene::PathTracing() {
  RTRTPathTracingShader({{3240, 2160}, ping_pong_.ping()->GetTexture("color"),
                         ping_pong_.ping()->GetTexture("position_ws"),
                         ping_pong_.ping()->GetTexture("surface_normal_ws")}, *this);
}

void RTRTScene::OutlierClamping() {
  RTRTOutlierClampingShader({{3240, 2160}, ping_pong_.ping()->GetTexture("color"),
                             ping_pong_.pong()->GetTexture("color")}, *this);
  ping_pong_.Swap();
}

void RTRTScene::Denoise() {

}

void RTRTScene::TemproalAccumulate() {

}