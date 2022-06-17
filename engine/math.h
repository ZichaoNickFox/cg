#pragma once

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#include <math.h>
#undef _USE_MATH_DEFINES
#else
#include <math.h>
#endif

#include <glm/glm.hpp>
#include <vector>

#include "engine/debug.h"

namespace util {

// util
float Lerp(float from, float to, float scale);
float Clamp(float value, float min, float max);

// sample
std::vector<glm::vec3> SampleSemishphere(int num);

// noise
std::vector<glm::vec3> Noise(int row_num, int col_num);

// random : 3 precisely
float RandFromTo(float from, float to);

namespace {
template<typename ElemType, typename LECompareFunc>
void QuickSelectInternal(std::vector<ElemType>* elems, int n, int l, int r, const LECompareFunc& le_compare_func) {
  int p = l;
  int s = l, t = r;
  while (s <= t) {
    while (s <= t && le_compare_func(elems->at(p), elems->at(t))) {
      t--;
    }
    if (s <= t) {
      std::swap(elems->at(p), elems->at(t));
      p = t;
    }
    while (s <= t && le_compare_func(elems->at(s), elems->at(p))) {
      s++;
    }
    if (s <= t) {
      std::swap(elems->at(s), elems->at(p));
      p = s;
    }
  }
  if (p == n) {
    return;
  } else if (p < n) {
    QuickSelectInternal(elems, n, p + 1, r, le_compare_func);
  } else {
    QuickSelectInternal(elems, n, l, p - 1, le_compare_func);
  }
}
}
template<typename ElemType, typename LECompareFunc>
int QuickSelect(std::vector<ElemType>* elems, int index, const LECompareFunc& le_compare_func) {
  CGCHECK(index >= 0 && index < elems->size()) << "index~" << index <<" size~" << elems->size();
  if (elems->size() == 1) {
    return elems->at(0);
  }
  QuickSelectInternal(elems, index, 0, elems->size() - 1, le_compare_func);
  return elems->at(index);
}

} // namespace util