#pragma once

#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/glm.hpp>

#include "base/math.h"
#include "renderer/camera.h"
#include "renderer/framebuffer.h"
#include "renderer/framebuffer_attachment.h"
#include "renderer/object.h"
#include "renderer/scene.h"
#include "renderer/shader.h"
#include "renderer/texture.h"

class DisneyScene : public cg::Scene {
 public:
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  std::vector<cg::ObjectMeta> object_metas_ = {
    
  };

  cg::Framebuffer fbo_;
};
