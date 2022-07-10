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

// ao : https://creativecoding.soe.ucsc.edu/courses/cmpm164/_schedule/AmbientOcclusion.pdf
// ssao : https://www.bilibili.com/video/BV1YK4y1T7yY?p=8 01:17:44
// hbao : https://developer.download.nvidia.cn/presentations/2008/SIGGRAPH/HBAO_SIG08b.pdf 
class SSAOScene : public renderer::Scene {
 public:
  void OnEnter() override;
  void OnUpdate() override;
  void OnRender() override;
  void OnExit() override;

 private:
  void Geometry();
  void SSAO();

  std::vector<renderer::ObjectMeta> object_metas_ = {
    {"plane", {glm::vec3(0, 0, 0), glm::quat(), glm::vec3(10, 1, 10)},
        "plane", "ruby"},
    {"cerberus", {glm::vec3(0, 0.4, 0), glm::angleAxis(float(M_PI) / 2, glm::vec3(1, 0, 0)), glm::vec3(0.02, 0.02, 0.02)},
        "cerberus", "cerberus"},
  };

  renderer::Framebuffer fbo_;
};
