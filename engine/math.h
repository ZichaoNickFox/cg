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

namespace engine {

// util
float lerp(float from, float to, float scale);

// sample
std::vector<glm::vec3> SampleSemishphere(int num);

// noise
std::vector<glm::vec3> Noise(int row_num, int col_num);

// random : 3 precisely
float RandFromTo(float from, float to);


} // namespace engine