#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace engine {
struct SphereData {
  glm::vec3 translation;
  glm::vec4 color;
  float radius;
};

struct Ray {
  glm::vec3 origin;
  glm::vec3 dirction;
};

class PathTracing {
 public:
  struct Option {
    int max_depth = 10;
    int sample_num = 4;
    int sample_in_unit_sphere = true;
  };
  PathTracing(const Option& option) : option_(option) {}
  glm::vec3 GetRayColor(Ray ray);

 private:
  Option option_;
};

};