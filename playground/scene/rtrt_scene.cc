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
  RTRTGeometryShader(const Scene& scene, const Object& object)
      : RenderShader(scene, "rtrt_geometry") {
    const Camera& camera = scene.camera();
    SetModel(object);
    SetCamera(camera);
    SetPrimitiveStartIndex(object);
    Run(scene, object);
  }
};

class RTRTPathTracingShader : public renderer::ComputeShader {
 public:
  struct Param {
    glm::vec2 resolution;
    renderer::Texture rasterized_position_ws;
    renderer::Texture rasterized_surface_normal_ws;
    renderer::Texture rasterized_primitive_index;
    renderer::Texture current_ping;
  };
  RTRTPathTracingShader(const Param& param, const Scene& scene) 
      : ComputeShader(scene, "rtrt_path_tracing") {
    SetCamera(scene.camera());

    SetTextureBinding({param.rasterized_position_ws, "rasterized_position_ws", GL_READ_ONLY});
    SetTextureBinding({param.rasterized_surface_normal_ws, "rasterized_surface_normal_ws", GL_READ_ONLY});
    SetTextureBinding({param.rasterized_primitive_index, "rasterized_primitive_index", GL_READ_ONLY});
    SetTextureBinding({param.current_ping, "current_ping", GL_WRITE_ONLY});

    SetFrameNum(scene);
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
    renderer::Texture ping_color;
    renderer::Texture pong_color;
    renderer::Texture texture_surface_normal;
  };
  RTRTDenoiseShader(const Param& param, const Scene& scene) 
      : ComputeShader(scene, "rtrt_denoise") {
    SetTextureBinding({param.ping_color, "texture_in", GL_READ_ONLY});
    SetTextureBinding({param.pong_color, "texture_out", GL_WRITE_ONLY});
    SetTextureBinding({param.texture_surface_normal, "texture_surface_normal", GL_READ_ONLY});
    SetWorkGroupNum({param.resolution.x / 32 + 1, param.resolution.y / 32 + 1, 1});
    SetResolution(param.resolution);
    Run();
  }
};

class RTRTTemproalAccumulationShader : public renderer::ComputeShader {
 public:
  struct Param {
    glm::vec2 resolution;
    renderer::Camera camera_1;
    renderer::Texture texture_position_ws;
    renderer::Texture current_ping;
    renderer::Texture last_ping;
    renderer::Texture last_pong;
  };
  RTRTTemproalAccumulationShader(const Param& param, const Scene& scene) 
      : ComputeShader(scene, "rtrt_temproal_accumulation") {
    SetCamera1(param.camera_1);
    SetTextureBinding({param.texture_position_ws, "texture_position_ws", GL_READ_ONLY});
    SetTextureBinding({param.current_ping, "current_ping", GL_READ_ONLY});
    SetTextureBinding({param.last_ping, "last_ping", GL_READ_ONLY});
    SetTextureBinding({param.last_pong, "last_pong", GL_WRITE_ONLY});
    SetWorkGroupNum({param.resolution.x / 32 + 1, param.resolution.y / 32 + 1, 1});
    SetResolution(param.resolution);
    SetFrameNum(scene);
    Run();
  }
};

void RTRTScene::OnEnter() {
  camera_->SetPerspectiveFov(60);
  camera_->SetTransform({{glm::vec3(0.286482, 0.708117, -1.065640)},
                          glm::quat(0.070520, {-0.005535, -0.994436, -0.078057}), {1, 1, 1}});

  glm::vec3 canvas_size(3240, 2160, 0);
  std::vector<renderer::FramebufferAttachment> fbo_attachments = {
    kAttachmentPositionWS, kAttachmentSurfaceNormalWS, kAttachmentPrimitiveIndex, kAttachmentDepth
  };
  fbo_.Init({canvas_size, fbo_attachments, kBlack});

  object_repo_.AddOrReplace(object_metas_);
  object_repo_.BreakIntoPrimitives(mesh_repo_, material_repo_, {}, &primitive_repo_);
  bvh_.Build(primitive_repo_, {100, BVH::Partition::kPos, 64});

  std::vector<glm::vec4> color(3240 * 2160, kBlack);
  current_frame1_ = renderer::CreateTexture2D(3240, 2160, color);
  current_frame2_ = renderer::CreateTexture2D(3240, 2160, color);
  last_frame1_ = renderer::CreateTexture2D(3240, 2160, color);
  last_frame2_ = renderer::CreateTexture2D(3240, 2160, color);

  current_frame_.Init(&current_frame1_, &current_frame2_);
  last_frame_.Init(&last_frame1_, &last_frame2_);

  camera_1_ = *camera_;

  glEnable_(GL_DEPTH_TEST);
}

void RTRTScene::OnUpdate() {
}

void RTRTScene::OnRender() {
  Rasterization();
  PathTracing();
  // OutlierClamping();
  // Denoise();
  TemproalAccumulate();
  FullscreenQuadShader({*last_frame_.pong()}, *this);
}

void RTRTScene::Rasterization() {
  fbo_.Bind();
  for (const Object& object : object_repo_.GetObjects()) {
    RTRTGeometryShader(*this, object);
  }
  fbo_.Unbind();
}

void RTRTScene::PathTracing() {
  RTRTPathTracingShader({{3240, 2160}, fbo_.GetTexture("position_ws"), fbo_.GetTexture("surface_normal_ws"),
                         fbo_.GetTexture("primitive_index"), *current_frame_.ping()}, *this);
}

void RTRTScene::OutlierClamping() {
  RTRTOutlierClampingShader({{3240, 2160}, *current_frame_.ping(), *current_frame_.pong()}, *this);
  current_frame_.Swap();
}

void RTRTScene::Denoise() {
  RTRTDenoiseShader({{3240, 2160}, *current_frame_.ping(), *current_frame_.pong(),
                     fbo_.GetTexture("surface_normal_ws")}, *this);
  current_frame_.Swap();
}

void RTRTScene::TemproalAccumulate() {
  RTRTTemproalAccumulationShader({{3240, 2160}, camera_1_, fbo_.GetTexture("position_ws"), 
                                  *current_frame_.ping(), *last_frame_.ping(), *last_frame_.pong()}, *this);
  camera_1_ = *camera_;
  last_frame_.Swap();
}

void RTRTScene::OnExit() {
}