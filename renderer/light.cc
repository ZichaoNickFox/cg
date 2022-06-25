#include "renderer/light.h"

#include "renderer/ssbo.h"
#include "renderer/util.h"

namespace renderer {

void LightRepo::UpdateSSBO() {
  bool dirty = !(dirty_lights_ == lights_);
  if (dirty) {
    std::vector<LightGPU> light_gpus = GetSSBOData();
    ssbo_.SetData(util::VectorSizeInByte(light_gpus), light_gpus.data());
    dirty_lights_ = lights_;
  }
}

std::vector<LightRepo::LightGPU> LightRepo::GetSSBOData() {
  std::vector<LightGPU> light_gpus(lights_.size());
  for (int i = 0; i < lights_.size(); ++i) {
    light_gpus[i].type_pos.x = lights_[i].type;
    light_gpus[i].type_pos.y = lights_[i].pos.x;
    light_gpus[i].type_pos.z = lights_[i].pos.y;
    light_gpus[i].type_pos.w = lights_[i].pos.z;
    light_gpus[i].color = lights_[i].color;
    light_gpus[i].attenuation_2_1_0 = glm::vec4(lights_[i].attenuation_2_1_0, 0);
  }
  return light_gpus;
}
} // namespace renderer