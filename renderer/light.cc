#include "renderer/light.h"

#include "renderer/ssbo.h"
#include "base/util.h"

namespace renderer {

Light::Light(Type in_type, int in_primitive_id) : type(in_type), primitive_index(in_primitive_id) {}

Light::Light(Type in_type, const glm::vec3& in_position, const glm::vec4& in_color, const glm::vec3& in_attenuation_2_1_0)
  : type(in_type), position(in_position), color(in_color), attenuation_2_1_0(in_attenuation_2_1_0) {}

void LightRepo::Add(const PrimitiveRepo& primitive_repo, const MaterialRepo& material_repo) {
  for (int i = 0; i < primitive_repo.data().size(); ++i) {
    const Primitive& primitive = primitive_repo.data()[i];
    const Material& material = material_repo.GetMaterial(primitive.material_index);
    if (material.emission != glm::vec4(0, 0, 0, 1)) {
      Light light(Light::kPrimitiveLight, i);
      lights_.push_back(light);
      primitive_light_num_++;
      primitive_light_area_ += primitive.triangle.GetArea();
    }
  }
  if (primitive_light_num_ > 0) {
    CGCHECK(primitive_light_area_ > 0) << " Primitive light area == 0";
  }
}

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
  type_primitiveindex = glm::vec4(light.type, light.primitive_index, 0, 0);
  color = light.color;
  attenuation_2_1_0 = glm::vec4(light.attenuation_2_1_0, 0.0);
}
} // namespace renderer