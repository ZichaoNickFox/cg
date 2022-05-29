#pragma once

namespace engine {

class PathTracing {
 public:
  struct Option {
    int max_depth = 10;
    int sample_num = 4;
    int sample_in_unit_sphere = true;
  };
};

};