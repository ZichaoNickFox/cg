#pragma once

#include <string>

#include "fmt/format.h"
#include <google/protobuf/text_format.h>
#include <glog/logging.h>

#include "engine/debug.h"

namespace util {
// file
void ReadFileToString(const std::string& path, std::string* content);
std::string GetFileExt(const std::string& file_name);

// string
bool EndsWith(const std::string& str, const std::string& end_with);
template<typename ...Args>
std::string Format(const char* fmt, const Args&... args) {
  return fmt::format(fmt, args...);
}

// proto
template<typename MessageType>
void ParseFromString(const std::string& content, MessageType* message) {
  BTCHECK(google::protobuf::TextFormat::ParseFromString(content, message));
}
template<typename ElemType>
std::vector<ElemType> ProtoRepeatedToVector(const google::protobuf::RepeatedPtrField<ElemType>& data) {
  return {data.begin(), data.end()};
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

}