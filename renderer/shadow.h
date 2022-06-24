#pragma once

#include <vector>

#include "renderer/ssbo.h"
#include "renderer/texture.h"

namespace renderer {

struct Shadow {
  glm::mat4 light_space_vp;
  int texture_depth;
};

struct ShadowRepo {
  void Add(const Shadow& shadow) { shadows_.push_back(shadow); }
  SSBO BindSSBO(int binding_point);

  int length() const { return shadows_.size(); }

  std::vector<Shadow> shadows_;
};

} // namespace renderer