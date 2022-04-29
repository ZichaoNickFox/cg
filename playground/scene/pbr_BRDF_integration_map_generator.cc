#include "playground/scene/pbr_BRDF_integration_map_generator.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "engine/transform.h"
#include "playground/object/sphere.h"
#include "playground/scene/common.h"
#include "playground/texture_repo.h"

void PbrBRDFIntegrationMapGenerator::OnEnter(Context *context) {
  engine::ColorFrameBuffer::Option option;
  option.clear_color = context->clear_color();
  option.mrt = 1;
  option.size = glm::ivec2{512, 512};
  color_frame_buffer_.Init(option);

  context->SetCamera(camera_);
}

void PbrBRDFIntegrationMapGenerator::OnUpdate(Context *context) {
  OnUpdateCommon _(context, "PbrBRDFIntegrationMapGenerator");
}

void PbrBRDFIntegrationMapGenerator::OnRender(Context *context) {
  color_frame_buffer_.Bind();
  EmptyObject empty_object;
  PbrBRDFIntegrationMapGeneratorShader({}, context, &empty_object);
  empty_object.OnRender(context);
  color_frame_buffer_.Unbind();

  context->SaveTexture("pbr_BRDF_integration_map", color_frame_buffer_.GetColorTexture());
  exit(0);
}

void PbrBRDFIntegrationMapGenerator::OnExit(Context *context) {
}