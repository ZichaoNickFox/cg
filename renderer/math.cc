#include "renderer/math.h"

#include <random>

#include "renderer/debug.h"
#include "renderer/util.h"

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

float RandFromTo(float from, float to) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distr(from * 1000, to * 1000);
  int res = distr(gen);
  return res / 1000.0;
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