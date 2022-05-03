#include "playground/scene/pbr_BRDF_integration_map_generator.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "engine/repo/texture_repo.h"
#include "engine/transform.h"
#include "playground/object/sphere.h"
#include "playground/scene/common.h"

constexpr int kLevel0Size = 512;

void PbrBRDFIntegrationMapGenerator::OnEnter(Context *context) {
  engine::ColorFrameBuffer::Option option;
  option.clear_color = context->clear_color();
  option.mrt = 1;
  option.size = glm::ivec2{kLevel0Size, kLevel0Size};
  color_frame_buffer_.Init(option);

  context->SetCamera(camera_);
}

void PbrBRDFIntegrationMapGenerator::OnUpdate(Context *context) {
  OnUpdateCommon _(context, "PbrBRDFIntegrationMapGenerator");
}

void PbrBRDFIntegrationMapGenerator::OnRender(Context *context) {
  engine::Texture2DData data(1, kLevel0Size * kLevel0Size * 4 * 4);

  color_frame_buffer_.Bind();
  EmptyObject empty_object;
  PbrBRDFIntegrationMapGeneratorShader({}, context, &empty_object);
  empty_object.OnRender(context);
  color_frame_buffer_.Unbind();

  *data.mutable_vector(0) = color_frame_buffer_.GetColorTextureData(0);

  engine::ResetTexture2DParam param{1, kLevel0Size, kLevel0Size, &data};
  context->ResetTexture2D("pbr_BRDF_integration_map", param);
  context->SaveTexture2D("pbr_BRDF_integration_map");
  exit(0);
}

void PbrBRDFIntegrationMapGenerator::OnExit(Context *context) {
}