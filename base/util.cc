#include "base/util.h"

#include <filesystem>
#include <fstream>
#include <limits>
#include <string>

#include "base/debug.h"

namespace util {

void ReadFileToString(const std::string& path, std::string* content) {
  CGCHECK(content);
  std::ifstream input_file(path);
  *content = std::string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
  input_file.close();
}

std::string FileExt(const std::string& file_name) {
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

std::string FileDir(const std::string& path) {
  std::string res = path;
  int slash_pos = path.find_last_of('/');
  if (slash_pos == std::string::npos) {
    return "";
  } else {
    return path.substr(0, slash_pos);
  }
}

std::string ReplaceBackslash(const std::string& path) {
  std::string res = path;
  std::replace(res.begin(), res.end(), '\\', '/');
  return res;
}

std::string FileJoin(const std::string& dir, const std::string& file) {
  if (dir.empty()) {
    return ReplaceBackslash(file);
  }
  if (file.empty()) {
    return ReplaceBackslash(dir);
  }
  if (dir.back() == '/' || dir.back() == '\\') {
    return ReplaceBackslash(dir + file);
  }
  return ReplaceBackslash(dir + "/" + file);
}

// S_IRWXU	00700权限，代表该文件所有者拥有读，写和执行操作的权限
// S_IRUSR(S_IREAD)	00400权限，代表该文件所有者拥有可读的权限
// S_IWUSR(S_IWRITE)	00200权限，代表该文件所有者拥有可写的权限
// S_IXUSR(S_IEXEC)	00100权限，代表该文件所有者拥有执行的权限
// S_IRWXG	00070权限，代表该文件用户组拥有读，写和执行操作的权限
// S_IRGRP	00040权限，代表该文件用户组拥有可读的权限
// S_IWGRP	00020权限，代表该文件用户组拥有可写的权限
// S_IXGRP	00010权限，代表该文件用户组拥有执行的权限
// S_IRWXO	00007权限，代表其他用户拥有读，写和执行操作的权限
// S_IROTH	00004权限，代表其他用户拥有可读的权限
// S_IWOTH	00002权限，代表其他用户拥有可写的权限
// S_IXOTH	00001权限，代表其他用户拥有执行的权限
void MakeDir(const std::string& dir) {
  if (dir.empty()) {
    return;
  }
  std::error_code error_code;
  std::filesystem::create_directories(dir, error_code);
  CGCHECK(!error_code) << error_code.message();
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

std::string TrimLeft(std::string str) {
  const size_t first = str.find_first_not_of(' ');
  if (first == std::string::npos) {
    return "";
  }
  return str.substr(first);
}

std::string TrimRight(std::string str) {
  const size_t last = str.find_last_not_of(' ');
  if (last == std::string::npos) {
    return "";
  }
  return str.substr(0, last + 1);
}

std::string Trim(std::string str) {
  return TrimRight(TrimLeft(str));
}

Time Now() {
  return std::chrono::high_resolution_clock::now();
}

int64_t DurationMillisecond(const Time& from, const Time& to) {
  std::chrono::high_resolution_clock::duration duration = to - from;
  return duration.count() / 1e6;
}

float AsFloat(const Time& time) {
  return time.time_since_epoch().count() / float(1e10);
}

int AsInt(const Time& time) {
  return time.time_since_epoch().count();
}

bool FloatEq(float value, float target) {
  return value < target + std::numeric_limits<float>::epsilon()
      && value > target - std::numeric_limits<float>::epsilon();
}

inline glm::ivec2 operator*(const glm::ivec2& left, double value) {
  return glm::ivec2(left.x * value, left.y * value);
}
} // namespace util
