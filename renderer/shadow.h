#pragma once

#include <vector>

#include "renderer/definition.h"
#include "renderer/ssbo.h"
#include "renderer/texture.h"

namespace renderer {

struct Shadow {
  glm::mat4 light_space_vp;
  int texture_depth;

  bool operator==(const Shadow& other) const {
    return memcmp(this, &other, sizeof(Shadow)) == 0;
  }
};

struct ShadowRepo {
  ShadowRepo() : ssbo_(SSBO_SHADOW_REPO) {}
  void Add(const Shadow& shadow) { shadows_.push_back(shadow); }
  void UpdateSSBO();

  int length() const { return shadows_.size(); }

  std::vector<Shadow> shadows_;
  std::vector<Shadow> dirty_shadows_;
  SSBO ssbo_;
};

} // namespace renderer