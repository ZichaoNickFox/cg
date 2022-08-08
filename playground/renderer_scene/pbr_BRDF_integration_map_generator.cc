#include "playground/renderer_scene/pbr_BRDF_integration_map_generator.h"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glog/logging.h"
#include "imgui.h"
#include <memory>

#include "renderer/repo/texture_repo.h"
#include "renderer/transform.h"
#include "playground/object/sphere_object.h"
#include "playground/renderer_scene/common.h"

constexpr int kLevel0Size = 512;

void PbrBRDFIntegrationMapGenerator::OnEnter(Scene *context) {
  cg::ColorFramebuffer::Option option;
  option.clear_color = context->clear_color();
  option.mrt = 1;
  option.size = glm::ivec2{kLevel0Size, kLevel0Size};
  color_framebuffer_.Init(option);

  context->SetCamera(camera_.get());
}

void PbrBRDFIntegrationMapGenerator::OnUpdate(Scene *context) {
  OnUpdateCommon _(context, "PbrBRDFIntegrationMapGenerator");
}

void PbrBRDFIntegrationMapGenerator::OnRender(Scene *context, int instance_num) {
  cg::Texture2DData data(1, kLevel0Size * kLevel0Size * 4 * 4);

  color_framebuffer_.Bind();
  EmptyObject empty_object;
  PbrBRDFIntegrationMapGeneratorShader({}, context, &empty_object);
  empty_object.OnRender(context);
  color_framebuffer_.Unbind();

  data.UpdateData(0, color_framebuffer_.GetColorTextureData(0));

  cg::TextureParam param{1, kLevel0Size, kLevel0Size, &data};
  context->ResetTexture2D("pbr_BRDF_integration_map", param);
  context->SaveTexture2D("pbr_BRDF_integration_map");
  exit(0);
}

void PbrBRDFIntegrationMapGenerator::OnExit(Scene *context) {
}