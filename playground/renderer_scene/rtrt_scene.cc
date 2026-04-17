#include "playground/renderer_scene/rtrt_scene.h"

#include "glm/gtx/string_cast.hpp"
#include <memory>

#include "base/color.h"
#include "base/math.h"
#include "renderer/inspector.h"
#include "renderer/mesh/lines_mesh.h"
#include "renderer/shader.h"
#include "renderer/transform.h"

using namespace cg;
using namespace cg;

class RTRTGeometryShader : public cg::RenderShader {
 public:
  RTRTGeometryShader(const Scene& scene, const Object& object)
      : RenderShader(scene, "rtrt_geometry") {
    const Camera& camera = scene.camera();
    SetModel(object);
    SetCamera(camera);
    SetPrimitiveStartIndex(object);
    Run(scene, object);
  }
};

class RTRTPathTracingShader : public cg::ComputeShader {
 public:
  struct Param {
    glm::vec2 resolution;
    cg::Texture rasterized_position_ws;
    cg::Texture rasterized_surface_normal_ws;
    cg::Texture rasterized_primitive_index;
    cg::Texture current_ping;
  };
  RTRTPathTracingShader(const Param& param, const Scene& scene) 
      : ComputeShader(scene, "rtrt_path_tracing") {
    SetCamera(scene.camera());

    SetTextureBinding({param.rasterized_position_ws, "rasterized_position_ws", TextureAccess::kReadOnly});
    SetTextureBinding({param.rasterized_surface_normal_ws, "rasterized_surface_normal_ws", TextureAccess::kReadOnly});
    SetTextureBinding({param.rasterized_primitive_index, "rasterized_primitive_index", TextureAccess::kReadOnly});
    SetTextureBinding({param.current_ping, "current_ping", TextureAccess::kWriteOnly});

    SetFrameNum(scene);
    SetWorkGroupNum({(param.resolution.x + 31) / 32, (param.resolution.y + 31) / 32, 1});
    SetResolution(param.resolution);
    Run();
  }
};

class RTRTOutlierClampingShader : public cg::ComputeShader {
 public:
  struct Param {
    glm::vec2 resolution;
    cg::Texture ping_color;
    cg::Texture pong_color;
  };
  RTRTOutlierClampingShader(const Param& param, const Scene& scene) 
      : ComputeShader(scene, "rtrt_outlier_clamping") {
    SetTextureBinding({param.ping_color, "texture_in", TextureAccess::kReadOnly});
    SetTextureBinding({param.pong_color, "texture_out", TextureAccess::kWriteOnly});
    SetWorkGroupNum({(param.resolution.x + 31) / 32, (param.resolution.y + 31) / 32, 1});
    SetResolution(param.resolution);
    Run();
  }
};

class RTRTDenoiseShader : public cg::ComputeShader {
 public:
  struct Param {
    glm::vec2 resolution;
    cg::Texture ping_color;
    cg::Texture pong_color;
    cg::Texture texture_surface_normal;
  };
  RTRTDenoiseShader(const Param& param, const Scene& scene) 
      : ComputeShader(scene, "rtrt_denoise") {
    SetTextureBinding({param.ping_color, "texture_in", TextureAccess::kReadOnly});
    SetTextureBinding({param.pong_color, "texture_out", TextureAccess::kWriteOnly});
    SetTextureBinding({param.texture_surface_normal, "texture_surface_normal", TextureAccess::kReadOnly});
    SetWorkGroupNum({(param.resolution.x + 31) / 32, (param.resolution.y + 31) / 32, 1});
    SetResolution(param.resolution);
    Run();
  }
};

class RTRTTemproalAccumulationShader : public cg::ComputeShader {
 public:
  struct Param {
    glm::vec2 resolution;
    cg::Camera camera_1;
    cg::Texture texture_position_ws;
    cg::Texture current_ping;
    cg::Texture last_ping;
    cg::Texture last_pong;
  };
  RTRTTemproalAccumulationShader(const Param& param, const Scene& scene) 
      : ComputeShader(scene, "rtrt_temproal_accumulation") {
    SetCamera1(param.camera_1);
    SetTextureBinding({param.texture_position_ws, "texture_position_ws", TextureAccess::kReadOnly});
    SetTextureBinding({param.current_ping, "current_ping", TextureAccess::kReadOnly});
    SetTextureBinding({param.last_ping, "last_ping", TextureAccess::kReadOnly});
    SetTextureBinding({param.last_pong, "last_pong", TextureAccess::kWriteOnly});
    SetWorkGroupNum({(param.resolution.x + 31) / 32, (param.resolution.y + 31) / 32, 1});
    SetResolution(param.resolution);
    SetFrameNum(scene);
    Run();
  }
};

void RTRTScene::OnEnter() {
  camera_->SetPerspectiveFov(60);
  camera_->SetTransform({{glm::vec3(0.286482, 0.708117, -1.065640)},
                          glm::quat(0.070520, {-0.005535, -0.994436, -0.078057}), {1, 1, 1}});

  const glm::ivec2 canvas_size = io_->framebuffer_size();
  std::vector<cg::FramebufferAttachment> fbo_attachments = {
    kAttachmentPositionWS, kAttachmentSurfaceNormalWS, kAttachmentPrimitiveIndex, kAttachmentDepth
  };
  fbo_.Init({canvas_size, fbo_attachments, kBlack});

  object_repo_.AddOrReplace(object_metas_);
  object_repo_.BreakIntoPrimitives(mesh_repo_, material_repo_, {}, &primitive_repo_);
  bvh_.Build(primitive_repo_, {100, BVH::Partition::kPos, 64});

  std::vector<glm::vec4> color(canvas_size.x * canvas_size.y, kBlack);
  current_frame1_ = cg::CreateTexture2D(canvas_size.x, canvas_size.y, color);
  current_frame2_ = cg::CreateTexture2D(canvas_size.x, canvas_size.y, color);
  last_frame1_ = cg::CreateTexture2D(canvas_size.x, canvas_size.y, color);
  last_frame2_ = cg::CreateTexture2D(canvas_size.x, canvas_size.y, color);

  current_frame_.Init(&current_frame1_, &current_frame2_);
  last_frame_.Init(&last_frame1_, &last_frame2_);

  camera_1_ = *camera_;

  rhi::GetDevice().SetDepthTestEnabled(true);
}

void RTRTScene::OnUpdate() {
}

void RTRTScene::OnRender() {
  Rasterization();
  PathTracing();
  // OutlierClamping();
  // Denoise();
  TemproalAccumulate();
  FullscreenQuadShader({*last_frame_.ping()}, *this);
}

void RTRTScene::Rasterization() {
  fbo_.Bind();
  for (const Object& object : object_repo_.GetObjects()) {
    RTRTGeometryShader(*this, object);
  }
  fbo_.Unbind();
}

void RTRTScene::PathTracing() {
  RTRTPathTracingShader({io().framebuffer_size(), fbo_.GetTexture("position_ws"), fbo_.GetTexture("surface_normal_ws"),
                         fbo_.GetTexture("primitive_index"), *current_frame_.ping()}, *this);
}

void RTRTScene::OutlierClamping() {
  RTRTOutlierClampingShader({io().framebuffer_size(), *current_frame_.ping(), *current_frame_.pong()}, *this);
  current_frame_.Swap();
}

void RTRTScene::Denoise() {
  RTRTDenoiseShader({io().framebuffer_size(), *current_frame_.ping(), *current_frame_.pong(),
                     fbo_.GetTexture("surface_normal_ws")}, *this);
  current_frame_.Swap();
}

void RTRTScene::TemproalAccumulate() {
  RTRTTemproalAccumulationShader({io().framebuffer_size(), camera_1_, fbo_.GetTexture("position_ws"), 
                                  *current_frame_.ping(), *last_frame_.ping(), *last_frame_.pong()}, *this);
  camera_1_ = *camera_;
  last_frame_.Swap();
}

void RTRTScene::OnExit() {
}
