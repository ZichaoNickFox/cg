#pragma once

#include <fmt/format.h>
#include <glm/glm.hpp>
#include <google/protobuf/text_format.h>
#include <string>
#include <vector>

#include "engine/debug.h"

namespace util {
// file
void ReadFileToString(const std::string& path, std::string* content);
std::string FileExt(const std::string& file_name);
std::string FileName(const std::string& path);
std::string FileDir(const std::string& path);
std::string ReplaceBackslash(const std::string& path);
void MakeDir(const std::string& dir);

// string
bool StartsWith(const std::string& str, const std::string& start_with);
bool EndsWith(const std::string& str, const std::string& end_with);
template<typename ...Args>
std::string Format(const char* format, const Args&... args) {
  return fmt::format(format, args...);
}
template<typename PtrType>
std::string AsString(PtrType* ptr) {
  return Format("{}", uint64_t(ptr));
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
uint64_t VectorSizeInByte(const std::vector<ElemType>& v) {
  return sizeof(ElemType) * v.size();
}
template<int N, typename ElemType>
std::array<ElemType, N> AsArray(const std::vector<ElemType>& in) {
  std::array<ElemType, N> res;
  std::copy_n(in.begin(), N, res.begin());
  return res;
}
template<typename Type>
const void* AsVoidPtr(const Type& var) {
  return reinterpret_cast<const void*>(var);
}
template<typename KeyType, typename ValueType>
std::vector<ValueType> AsValueVector(const std::unordered_map<KeyType, ValueType>& in) {
  std::vector<ValueType> res;
  for (auto& pair : in) {
    res.push_back(pair.second);
  }
  return res;
}
template<typename KeyType, typename ValueType>
std::vector<KeyType> AsKeyVector(const std::unordered_map<KeyType, ValueType>& in) {
  std::vector<KeyType> res;
  for (auto& pair : in) {
    res.push_back(pair.first);
  }
  return res;
}
template <typename ArrayElemType, int N>
std::vector<ArrayElemType> AsVector(const ArrayElemType (&array)[N]) {   
 int num = (sizeof(array) / sizeof(array[0])); 
 return {array, array + num};
};
template<typename VectorElemType>
std::vector<VectorElemType> SubVector(const std::vector<VectorElemType>& source, int begin, int end) {
  return {source.begin() + begin, source.begin() + end};
}
template<typename VectorElemType>
void Remove(const VectorElemType& elem, std::vector<VectorElemType>* elems) {
  for (std::vector<VectorElemType>::iterator iter = elems->begin(); iter < elems->end();) {
    if (elem == *iter) {
      iter = elems->erase(iter);
    } else {
      iter++;
    }
  }
}
template<typename ElemType>
void VectorOverride(std::vector<ElemType>* dist, int dist_begin,
                    const std::vector<ElemType>& src, int src_begin, int src_end) {
  int len = src_end - src_begin; 
  if (dist->size() - dist_begin < len) {
    dist->resize(dist_begin + len);
  }
  memcpy(dist->data() + dist_begin, src.data() + src_begin, len * sizeof(ElemType));
}

// time
using Time = std::chrono::high_resolution_clock::time_point;
Time Now();
int64_t DurationMillisecond(const Time& from, const Time& to);
float AsFloat(const Time& time);
int AsInt(const Time& time);

// float
float FloatEq(float value, float target);

// glm
glm::ivec2 operator*(const glm::ivec2& left, double value);

}