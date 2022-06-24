#include "renderer/light.h"

#include "renderer/ssbo.h"
#include "renderer/util.h"

namespace renderer {

void LightRepo::BindSSBO(int binding_point) {
  std::vector<LightGPU> light_gpus(lights_.size());
  for (int i = 0; i < lights_.size(); ++i) {
    light_gpus[i].type_pos.x = lights_[i].type;
    light_gpus[i].type_pos.y = lights_[i].pos.x;
    light_gpus[i].type_pos.z = lights_[i].pos.y;
    light_gpus[i].type_pos.w = lights_[i].pos.z;
    light_gpus[i].color = lights_[i].color;
    light_gpus[i].attenuation_2_1_0.x = lights_[i].attenuation_quadratic;
    light_gpus[i].attenuation_2_1_0.y = lights_[i].attenuation_linear;
    light_gpus[i].attenuation_2_1_0.z = lights_[i].attenuation_constant;
    light_gpus[i].attenuation_2_1_0.w = 0;
  }
  ssbo_.Init(binding_point, util::VectorSizeInByte(light_gpus), light_gpus.data());
}

} // namespace renderer