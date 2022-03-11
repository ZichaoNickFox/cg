#pragma once

#include <string>

#include "fmt/format.h"

namespace string_util {

bool EndsWith(const std::string& str, const std::string& end_with);

template<typename ...Args>
std::string Format(const char* fmt, const Args&... args) {
  return fmt::format(fmt, args...);
}

}