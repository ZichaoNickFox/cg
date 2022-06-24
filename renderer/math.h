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

#include "renderer/debug.h"

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
void QuickSelectInternal(std::vector<ElemType>* elems, int begin, int end, int n, const LECompareFunc& le_compare_func) {
  int p = begin;
  int s = begin, t = end - 1;
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
    QuickSelectInternal(elems, p + 1, end, n, le_compare_func);
  } else {
    QuickSelectInternal(elems, begin, p, n, le_compare_func);
  }
}
}
template<typename ElemType, typename LECompareFunc>
int QuickSelect(std::vector<ElemType>* elems, int begin, int end, int index, const LECompareFunc& le_compare_func) {
  CGCHECK(begin >= 0 && end <= elems->size()) << " begin~" << begin << " end~" << end << " size" << elems->size();
  CGCHECK(index >= begin && index < end) << " begin~" << begin << " end~" << end << " index~" << index;
  if (end - begin == 1) {
    return begin;
  }
  QuickSelectInternal(elems, begin, end, index, le_compare_func);
  return index;
}

} // namespace util