#pragma once

#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/glm.hpp>

#include "renderer/camera.h"
#include "renderer/framebuffer.h"
#include "renderer/framebuffer_attachment.h"
#include "renderer/math.h"
#include "renderer/object.h"
#include "renderer/scene.h"
#include "renderer/shader.h"
#include "renderer/texture.h"

class DisneyScene : public renderer::Scene {
 public:
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  std::vector<renderer::ObjectMeta> object_metas_ = {
    
  };

  renderer::Framebuffer fbo_;
};
