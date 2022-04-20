#include "playground/util.h"

#include <fstream>
#include <glog/logging.h>
#include <limits>
#include <string>

namespace util {

void ReadFileToString(const std::string& path, std::string* content) {
  CGCHECK(content);
  std::ifstream input_file(path);
  *content = std::string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
  input_file.close();
}

std::string GetFileExt(const std::string& file_name) {
  int pos = file_name.find_last_of('.');
  if (pos == std::string::npos) {
    return "";
  }
  return file_name.substr(pos + 1);
}

std::string FileName(const std::string& path) {
  std::string res = path;
  int slash_pos = path.find_last_of('/');
  if (slash_pos != std::string::npos) {
    res = path.substr(slash_pos + 1);
  }
  int dot_pos = res.find_last_of('.');
  if (dot_pos != std::string::npos) {
    res = res.substr(0, dot_pos);
  }
  return res;
}

bool StartsWith(const std::string& str, const std::string& start_with) {
  return str.rfind(start_with, 0) == 0;
}

bool EndsWith(const std::string& str, const std::string& end_with) {
  if (str.length() >= end_with.length()) {
    return (0 == str.compare (str.length() - end_with.length(), end_with.length(), end_with));
  } else {
    return false;
  }
}

Time Now() {
  return std::chrono::high_resolution_clock::now();
}

int64_t DurationMillisecond(const Time& from, const Time& to) {
  std::chrono::high_resolution_clock::duration duration = to - from;
  return duration.count() / 1e6;
}

namespace {
int RandFromTo(int from, int to) {
  CHECK(to > from) << "to LE than from";
  std::srand(Now().time_since_epoch().count());
  uint slide = std::max(0, 0 - from);
  uint rand_from = from + slide;
  uint rand_to = to + slide;
  uint rand_value = rand_from + uint(std::rand()) % (rand_to - rand_from + 1);
  return rand_value - slide;
}
}

float RandFromTo(float from, float to) {
  int rand_from = from * 1000;
  int rand_to = to * 1000;
  int rand_value = RandFromTo(rand_from, rand_to);
  return rand_value / 1000.0f;
}

float FloatEq(float value, float target) {
  return value < target + std::numeric_limits<float>::epsilon()
      && value > target - std::numeric_limits<float>::epsilon();
}
} // namespace util