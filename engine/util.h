#pragma once

#include <string>

#include "fmt/format.h"
#include <glm/glm.hpp>
#include <glog/logging.h>
#include <google/protobuf/text_format.h>

#include "engine/debug.h"

namespace util {
// file
void ReadFileToString(const std::string& path, std::string* content);
std::string FileExt(const std::string& file_name);
std::string FileName(const std::string& path);
std::string FileDir(const std::string& path);
void MakeDir(const std::string& dir);

// string
bool StartsWith(const std::string& str, const std::string& start_with);
bool EndsWith(const std::string& str, const std::string& end_with);
template<typename ...Args>
std::string Format(const char* fmt, const Args&... args) {
  return fmt::format(fmt, args...);
}

// proto
template<typename MessageType>
void ParseFromString(const std::string& content, MessageType* message) {
  CGCHECK(google::protobuf::TextFormat::ParseFromString(content, message));
}
template<typename ElemType>
std::vector<ElemType> ProtoRepeatedToVector(const google::protobuf::RepeatedPtrField<ElemType>& data) {
  return {data.begin(), data.end()};
}
template<typename KeyType, typename ValueType>
std::unordered_map<KeyType, ValueType> ProtoMap2UnorderedMap(const google::protobuf::Map<KeyType, ValueType>& map) {
  std::unordered_map<KeyType, ValueType> res;
  for (auto& p : map) {
    res[p.first] = p.second;
  }
  return res;
}

// stl
template<typename ElemType>
int VectorByteSize(const std::vector<ElemType>& v) {
  return sizeof(ElemType) * v.size();
}

// time
using Time = std::chrono::high_resolution_clock::time_point;
Time Now();
int64_t DurationMillisecond(const Time& from, const Time& to);

// random : 3 precisely
float RandFromTo(float from, float to);

float FloatEq(float value, float target);

// glm
glm::ivec2 operator*(const glm::ivec2& left, double value);

}