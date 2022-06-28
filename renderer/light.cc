#include "renderer/light.h"

#include "renderer/ssbo.h"
#include "renderer/util.h"

namespace renderer {

void LightRepo::UpdateSSBO() {
  bool dirty = !(dirty_lights_ == lights_);
  if (dirty) {
    std::vector<LightGPU> light_gpus;
    std::transform(lights_.begin(), lights_.end(), std::back_inserter(light_gpus),
                   [] (const Light& light) { return LightGPU(light); });
    ssbo_.SetData(util::VectorSizeInByte(light_gpus), light_gpus.data());
    dirty_lights_ = lights_;
  }
}

LightRepo::LightGPU::LightGPU(const Light& light) {
  type_pos = glm::vec4(light.type, light.pos);
  color = light.color;
  attenuation_2_1_0 = glm::vec4(light.attenuation_2_1_0, 0.0);
}
} // namespace renderer