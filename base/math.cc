#include "base/math.h"

#include <random>

#include "base/debug.h"

namespace math {

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
  std::uniform_int_distribution<> distribution(from * 1000, to * 1000);
  int res = distribution(gen);
  return res / 1000.0;
}

std::vector<glm::vec4> SampleSemishphere(int num) {
  std::vector<glm::vec4> res(num);
  for (int i = 0; i < num; ++i) {
    glm::vec3 sample(RandFromTo(-1, 1), RandFromTo(-1, 1), RandFromTo(0, 1));
    sample = glm::normalize(sample);
    float scale = float(i) / num;
    scale = Lerp(0.1, 1.0, scale * scale);
    sample = sample * RandFromTo(0, 1) * scale;
    res[i] = glm::vec4(sample, 1.0);
  }
  return res;
}

std::vector<glm::vec4> Noise(int row_num, int col_num) {
  std::vector<glm::vec4> res(row_num * col_num);
  for (int i = 0; i < row_num; ++i) {
    for (int j = 0; j < col_num; ++j) {
      glm::vec3 noice(RandFromTo(-1, 1), RandFromTo(-1, 1), 0.0);
      res[i * col_num + j] = glm::vec4(noice, 1.0);
    }
  }
  return res;
}

std::vector<float> Linspace(float from, float to, float interval) {
  std::vector<float> res;
  for (float iter = from; iter < to; iter += interval) {
    res.push_back(iter);
  }
  return res;
}

std::vector<float> Transform(const std::vector<float>& vs, const std::function<float(float)>& func) {
  std::vector<float> res(vs.size());
  for (int i = 0; i < vs.size(); ++i) {
    res[i] = func(vs[i]);
  }
  return res;
}

} // namespace math