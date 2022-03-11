#include "engine/string_util.h"

namespace string_util
{
bool EndsWith(const std::string& str, const std::string& ending) {
  if (str.length() >= ending.length()) {
    return (0 == str.compare (str.length() - ending.length(), ending.length(), ending));
  } else {
    return false;
  }
}

} // namespace  string_util
