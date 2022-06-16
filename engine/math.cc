#include "engine/math.h"

#include "engine/debug.h"
#include "engine/util.h"

namespace util {

float Lerp(float from, float to, float scale) {
  return from + (to - from) * scale;
}

float Clamp(float value, float min, float max) {
  if (value <= min) {
    return min;
  } else if (value >= max) {
    return max;
  } else {
    return value;
  }
}

namespace {
int RandFromToInternal(int from, int to) {
  CHECK(to > from) << "to LE than from";
  std::srand(util::Now().time_since_epoch().count());
  uint32_t slide = std::max(0, 0 - from);
  uint32_t rand_from = from + slide;
  uint32_t rand_to = to + slide;
  uint32_t rand_value = rand_from + uint32_t(std::rand()) % (rand_to - rand_from + 1);
  return rand_value - slide;
}
}

float RandFromTo(float from, float to) {
  int rand_from = from * 1000;
  int rand_to = to * 1000;
  int rand_value = RandFromToInternal(rand_from, rand_to);
  return rand_value / 1000.0f;
}

std::vector<glm::vec3> SampleSemishphere(int num) {
  std::vector<glm::vec3> res(num);
  for (int i = 0; i < num; ++i) {
    glm::vec3 sample(RandFromTo(-1, 1), RandFromTo(-1, 1), RandFromTo(0, 1));
    sample = glm::normalize(sample);
    float scale = float(i) / num;
    scale = Lerp(0.1, 1.0, scale * scale);
    sample = sample * util::RandFromTo(0, 1) * scale;
    res[i] = sample;
  }
  return res;
}

std::vector<glm::vec3> Noise(int row_num, int col_num) {
  std::vector<glm::vec3> res(row_num * col_num);
  for (int i = 0; i < row_num; ++i) {
    for (int j = 0; j < col_num; ++j) {
      glm::vec3 noice(util::RandFromTo(-1, 1), util::RandFromTo(-1, 1), 0.0);
      res[i * col_num + j] = noice;
    }
  }
  return res;
}
} // namespace util